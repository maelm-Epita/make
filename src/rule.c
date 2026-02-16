#define _GNU_SOURCE

#include "rule.h"

#include <stdio.h>
#include <string.h>

#include "codes.h"
#include "pattern.h"
#include "rule_vector.h"

static int contains(char *line, char c)
{
    for (size_t i = 0; line[i] != 0; i++)
    {
        if (line[i] == c)
        {
            return 1;
        }
    }
    return 0;
}

static int is_pattern_target(struct rule *r)
{
    if (r == NULL)
    {
        return 0;
    }
    return contains(r->target, '%');
}

char *get_first_dependency_target(struct rule *r)
{
    if (r == NULL)
    {
        return NULL;
    }
    char *dep = string_vector_get(r->dependencies, 0);
    return dep == NULL ? "" : dep;
}

char *get_all_dependency_target(struct rule *r)
{
    if (r == NULL)
    {
        return NULL;
    }
    if (r->dependencies == NULL || r->dependencies->data == NULL
        || r->dependencies->size == 0)
    {
        return strdup("");
    }
    char *str;
    if (asprintf(&str, "%s", r->dependencies->data[0]) == -1)
    {
        ERR("_internal error: Failed to allocate memory");
        return "";
    }
    for (size_t i = 1; i < r->dependencies->size; i++)
    {
        char *new_str;
        if (asprintf(&new_str, "%s %s", str, r->dependencies->data[i]) == -1)
        {
            free(str);
            ERR("_internal error: Failed to allocate memory");
            return "";
        }
        free(str);
        str = new_str;
    }
    return str;
}

struct rule *get_first_standard_target(struct rule_vector *rules)
{
    if (rules == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < rules->size; i++)
    {
        struct rule *r = rule_vector_get(rules, i);
        if (r == NULL)
        {
            return NULL;
        }
        if (!is_pattern_target(r) && strcmp(r->target, ".PHONY") != 0)
        {
            return r;
        }
    }
    return NULL;
}

struct rule *find_rule_with_target(struct rule_vector *rules, char *target)
{
    if (rules == NULL || target == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < rules->size; i++)
    {
        struct rule *r = rule_vector_get(rules, i);
        if (r == NULL)
        {
            return NULL;
        }
        if (strcmp(r->target, target) == 0)
        {
            return r;
        }
    }
    return NULL;
}

struct rule *find_rule_matching_target(struct rule_vector *rules, char *target,
                                       char **out_glob)
{
    if (rules == NULL || target == NULL)
    {
        return NULL;
    }
    struct rule *smallest_glob_rule = NULL;
    char *smallest_glob = NULL;
    for (size_t i = 0; i < rules->size; i++)
    {
        struct rule *r = rule_vector_get(rules, i);
        if (r == NULL)
        {
            return NULL;
        }
        char *glob = NULL;
        if (satisfies_pattern(target, r->target, &glob))
        {
            if (glob == NULL)
            {
                if (smallest_glob != NULL)
                {
                    free(smallest_glob);
                }
                smallest_glob_rule = r;
                smallest_glob = glob;
                break;
            }
            if (smallest_glob == NULL)
            {
                smallest_glob_rule = r;
                smallest_glob = glob;
            }
            else if (strlen(glob) < strlen(smallest_glob))
            {
                free(smallest_glob);
                smallest_glob_rule = r;
                smallest_glob = glob;
            }
            else
            {
                free(glob);
            }
        }
    }
    if (out_glob != NULL)
    {
        *out_glob = smallest_glob;
    }
    else
    {
        free(smallest_glob);
    }
    return smallest_glob_rule;
}

int is_phony(struct rule_vector *rules, char *target)
{
    struct rule *phony = find_rule_with_target(rules, ".PHONY");
    if (phony == NULL)
    {
        return 0;
    }
    for (size_t i = 0; i < phony->dependencies->size; i++)
    {
        if (strcmp(target, phony->dependencies->data[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

struct rule *rule_init(void)
{
    struct rule *rule = malloc(sizeof(struct rule));
    if (rule == NULL)
    {
        return NULL;
    }
    rule->dependencies = NULL;
    rule->recipe = NULL;
    rule->target = NULL;
    return rule;
}

void rule_free(struct rule *r)
{
    string_vector_free(r->dependencies);
    if (r->target != NULL)
    {
        free(r->target);
    }
    if (r->recipe != NULL)
    {
        string_vector_free(r->recipe);
    }
    free(r);
}
