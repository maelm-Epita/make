#define _GNU_SOURCE

#include "expansion.h"

#include <stdio.h>
#include <string.h>

#include "codes.h"

#define MERR_RET                                                               \
    {                                                                          \
        ERR("_internal error: Failed to allocate memory");                     \
        *err = 1;                                                              \
        return input->line;                                                    \
    }

static inline int is_reference_opening(char c)
{
    return c == '(' || c == '{';
}

static inline int is_reference_closing(char c)
{
    return c == ')' || c == '}';
}

/*
$a variable is a
$( we step until ) and that's our variable
*/

// $(haha$(abcde)$($(yoyo)))
// $(haha$a$($(yoyo)))
// $) is valid but $( is not
// THIS IS A$aTST
// THIS IS A$(a)TST
//
// starti = $, endi = a
// starti = $, endi = )
static int find_variable_index_name(char *str, struct span *span,
                                    int is_var_definition)
{
    if (str[0] == 0)
    {
        return 1;
    }
    size_t i = 0;
    int starti = -1;
    int endi = -1;
    while (str[i] != 0)
    {
        if (str[i] == '=' && is_var_definition)
        {
            break;
        }
        if (str[i] == '$')
        {
            if (str[i + 1] == 0 || str[i + 1] == '\n')
            {
                i++;
                continue;
            }
            else if (str[i + 1] == '$')
            {
                i += 2;
                continue;
            }
            else
            {
                starti = i;
                if (!is_reference_opening(str[i + 1]))
                {
                    endi = i + 1;
                    break;
                }
            }
        }
        else if (is_reference_closing(str[i]))
        {
            if (starti == -1)
            {
                i++;
                continue;
            }
            else
            {
                endi = i;
            }
            break;
        }
        i++;
    }
    if (endi == -1)
    {
        if (starti != -1)
        {
            ERR("unterminated variable reference")
            return -1;
        }
        return 1;
    }
    span->start = starti;
    span->end = endi;
    return 0;
}

static const char *get_var(struct expansion_input *input, int *err,
                           struct span first_inner_v)
{
    const char *val = NULL;
    char *varname = NULL;
    input->line[first_inner_v.start] = 0;
    if (is_reference_opening(input->line[first_inner_v.start + 1]))
    {
        input->line[first_inner_v.end] = 0;
        varname = strdup(input->line + first_inner_v.start + 2);
    }
    else
    {
        if (asprintf(&varname, "%c", input->line[first_inner_v.end]) == -1)
        {
            MERR_RET;
        }
    }
    if (strcmp(varname, "$") == 0)
    {
        val = "$";
    }
    if (val == NULL)
    {
        val = hash_map_get(input->variables, varname);
    }
    if (val == NULL)
    {
        val = getenv(varname);
    }
    free(varname);
    if (val == NULL)
    {
        val = "";
    }
    return val;
}

static char *expand_variable(struct expansion_input *input, size_t *out_bytes,
                             int *err, struct span first_inner_v)
{
    const char *val = get_var(input, err, first_inner_v);
    char *pre = input->line;
    char *post = input->line + first_inner_v.end + 1;
    char *new_str = NULL;
    int bytes = asprintf(&new_str, "%s%s%s", pre, val, post);
    if (bytes == -1)
    {
        MERR_RET;
    }
    *out_bytes = bytes;
    free(input->line);
    *err = 0;
    return new_str;
}

static char *expand_double_dollar(struct expansion_input *input,
                                  size_t *out_bytes, int *err)
{
    size_t i = 0;
    while (input->line[i] != 0)
    {
        if (input->line[i] == '$' && input->line[i + 1] == '$')
        {
            break;
        }
        i++;
    }
    if (input->line[i] == 0)
    {
        return input->line;
    }
    input->line[i] = 0;
    char *pre = input->line;
    char *post = input->line + i + 1;
    char *new_str = NULL;
    int bytes = asprintf(&new_str, "%s%s", pre, post);
    if (bytes == -1)
    {
        MERR_RET;
    }
    *out_bytes = bytes;
    free(input->line);
    *err = 0;
    return new_str;
}

char *expand_variables(struct expansion_input *input, size_t *out_bytes,
                       int *err)
{
    struct span first_inner_v = { 0 };
    int r = find_variable_index_name(input->line, &first_inner_v,
                                     input->variable_definition);
    // parsing error unmatched $(
    if (r == -1)
    {
        *err = 1;
        return input->line;
    }
    // nothing to expand
    else if (r == 1)
    {
        input->line = expand_double_dollar(input, out_bytes, err);
        *err = 0;
        return input->line;
    }
    // should expand
    input->line = expand_variable(input, out_bytes, err, first_inner_v);
    return expand_variables(input, out_bytes, err);
}
