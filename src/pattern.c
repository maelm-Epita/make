#define _GNU_SOURCE

#include "pattern.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "codes.h"

static int aux_satisfies_pattern(char *str, char *pattern, char **out_glob,
                                 struct pattern_helpers *helpers)
{
    if (*str == 0 && *pattern == 0)
    {
        if (*out_glob != NULL)
        {
            (*out_glob)[helpers->glob_i] = 0;
        }
        return 1;
    }
    if (*pattern == '%' && !helpers->foundglob)
    {
        if (*out_glob == NULL)
        {
            *out_glob = strdup(str);
            if (*out_glob == NULL)
            {
                ERR("_internal error: Failed to allocate memory");
                return 0;
            }
        }
        if (*(pattern + 1) == 0)
        {
            return 1;
        }
        else
        {
            helpers->foundglob = 1;
            if (aux_satisfies_pattern(str, pattern + 1, out_glob, helpers))
            {
                return 1;
            }
            else if (*str == 0)
            {
                return 0;
            }
            helpers->glob_i++;
            helpers->foundglob = 0;
            return aux_satisfies_pattern(str + 1, pattern, out_glob, helpers);
        }
    }
    else
    {
        if (*pattern != *str)
        {
            return 0;
        }
        else
        {
            return aux_satisfies_pattern(str + 1, pattern + 1, out_glob,
                                         helpers);
        }
    }
}

int satisfies_pattern(char *str, char *pattern, char **out_glob)
{
    if (str == NULL || pattern == NULL)
    {
        return 0;
    }
    *out_glob = NULL;
    struct pattern_helpers helpers = { 0, 0 };
    int satisfies = aux_satisfies_pattern(str, pattern, out_glob, &helpers);
    if (!satisfies && *out_glob != NULL)
    {
        free(*out_glob);
    }
    return satisfies;
}

char *replace_glob(char *glob, char *pattern)
{
    char *pattern_dup = strdup(pattern);
    if (pattern_dup == NULL)
    {
        return NULL;
    }
    if (glob == NULL)
    {
        return pattern_dup;
    }
    char *pre = pattern_dup;
    size_t i = 0;
    while (pattern_dup[i] != 0)
    {
        if (pattern_dup[i] == '%')
        {
            break;
        }
        i++;
    }
    if (pattern_dup[i] != '%')
    {
        return pattern_dup;
    }
    pattern_dup[i] = 0;
    char *post = (pattern_dup + i + 1);
    char *new_str = NULL;
    int bytes = asprintf(&new_str, "%s%s%s", pre, glob, post);
    if (bytes == -1)
    {
        return NULL;
    }
    free(pattern_dup);
    return new_str;
}
