#define _POSIX_C_SOURCE 200809L

#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "codes.h"
#include "expansion.h"
#include "helpers.h"
#include "rule_vector.h"

enum PREV_LINE_RULE
{
    PREV_NOT_RULE,
    PREV_RULE,
    PREV_SKIP_RULE,
};

static int is_variable_definition(char *line)
{
    for (size_t i = 0; line[i] != 0; i++)
    {
        if (line[i] == ':')
        {
            return 0;
        }
        if (line[i] == '#')
        {
            return 0;
        }
        else if (line[i] == '=')
        {
            return 1;
        }
    }
    return 0;
}

static int is_valid_name(char *name)
{
    for (size_t i = 0; name[i] != 0; i++)
    {
        char c = name[i];
        if (isblank(c) || c == ':' || c == '=' || c == '#')
        {
            return 0;
        }
    }
    return 1;
}

static char *remove_comments(char *line)
{
    size_t i = 0;
    while (line[i] != 0)
    {
        if (line[i] == '#')
        {
            line[i] = '\n';
            i++;
            break;
        }
        i++;
    }
    line[i] = 0;
    return line;
}

static int parse_var(char *line, struct hash_map *variables)
{
    if (line[0] == '=')
    {
        ERR("empty variable name");
        return 1;
    }
    char *key = strtok(line, "=");
    key = trim(key);
    if (!is_valid_name(key))
    {
        ERR("invalid variable name");
        return 1;
    }
    char *value = strtok(NULL, "\n");
    if (value == NULL)
    {
        ERR("variable definition with no value");
        return 1;
    }
    value = trim_l(value);
    char *cpy_key = strdup(key);
    char *cpy_value = strdup(value);
    if (cpy_key[0] == 0)
    {
        ERR("empty variable name");
        free(cpy_key);
        free(cpy_value);
        return 1;
    }
    hash_map_insert(variables, cpy_key, cpy_value);
    return 0;
}

static int parse_target_dep(char *line, struct rule_vector *rules)
{
    struct rule *rule = rule_init();
    char *target = strtok(line, ":");
    target = trim(target);
    if (!is_valid_name(target))
    {
        ERR("invalid target name");
        rule_free(rule);
        return 1;
    }
    char *cpy_target = strdup(target);
    rule->target = cpy_target;
    struct string_vector *dependencies = string_vector_init();
    if (dependencies == NULL)
    {
        rule_free(rule);
        return 1;
    }
    char *dep;
    while ((dep = strtok(NULL, "\n\t ")) != NULL)
    {
        if (!is_valid_name(dep))
        {
            ERR("invalid dependency name");
            rule_free(rule);
            string_vector_free(dependencies);
            return 1;
        }
        char *cpy_dep = strdup(dep);
        if (string_vector_push_back(dependencies, cpy_dep) != 0)
        {
            ERR("_internal error: Failed to allocate memory");
            return 1;
        }
    }
    rule->dependencies = dependencies;
    if (rule_vector_push_back(rules, rule) != 0)
    {
        ERR("_internal error: Failed to allocate memory");
        free(rule);
        return 1;
    }
    return 0;
}

static int parse_recipe(char *line, struct rule *rule)
{
    line = strtok(line, "\n");
    char *line_cpy = strdup(line);
    if (rule->recipe == NULL)
    {
        rule->recipe = string_vector_init();
    }
    if (string_vector_push_back(rule->recipe, line_cpy) != 0)
    {
        ERR("_internal error: Failed to allocate memory");
        free(line_cpy);
        return 1;
    }
    return 0;
}

// 39 LINES FUNCTION
int parse_line(char **out_line, size_t *out_bytes, struct parsed_make *out,
               int *rule_line)
{
    char *line = *out_line;
    if (line[0] == '\t')
    {
        if (is_blank_line_ignore_comment(line))
        {
            return 0;
        }
        if ((*rule_line) == PREV_NOT_RULE)
        {
            ERR("found command outside of rule");
            return 1;
        }
        if ((*rule_line) == PREV_SKIP_RULE)
        {
            return 0;
        }
        line = trim(line);
        struct rule *cur_rule =
            rule_vector_get(out->rules, rule_vector_size(out->rules) - 1);
        return parse_recipe(line, cur_rule);
    }
    else
    {
        int variable_definition = is_variable_definition(line);
        struct expansion_input input = { line, out->variables,
                                         variable_definition };
        int err = 0;
        line = expand_variables(&input, out_bytes, &err);
        *out_line = line;
        if (err)
        {
            return 1;
        }
        line = trim(line);
        line = remove_comments(line);
        // expand variables before trimming, the line "all: $ " should expand to
        // "all: " and not "all: $"
        if (contains(line, ':'))
        {
            if (line[0] == ':')
            {
                *rule_line = PREV_SKIP_RULE;
                return 0;
            }
            else
            {
                *rule_line = PREV_RULE;
                return parse_target_dep(line, out->rules);
            }
        }
        else if (variable_definition)
        {
            *rule_line = PREV_NOT_RULE;
            return parse_var(line, out->variables);
        }
        else
        {
            if (!is_blank_line(line))
            {
                ERR("missing separator");
                return 1;
            }
            return 0;
        }
    }
}

int parse(FILE *makefile, struct parsed_make *out)
{
    struct hash_map *variables = hash_map_init();
    if (variables == NULL)
    {
        return 2;
    }
    out->variables = variables;
    struct rule_vector *rules = rule_vector_init();
    if (rules == NULL)
    {
        return 2;
    }
    out->rules = rules;
    size_t a = 0;
    char *line = NULL;
    int rule_line = 0;
    while (getline(&line, &a, makefile) != -1)
    {
        if (parse_line(&line, &a, out, &rule_line) != 0)
        {
            free(line);
            return 2;
        }
    }
    free(line);
    return 0;
}
