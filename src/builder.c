#define _GNU_SOURCE

#include "builder.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "codes.h"
#include "expansion.h"
#include "helpers.h"
#include "pattern.h"
#include "rule.h"

enum UTD_NBD_RETURNS
{
    UTD_FALSE = 0,
    UTD_TRUE = 1,
    UTD_ERR = 2,
    UTD_SPECIAL = 3,
    UTDNBD_BUILD = 4,
    UTDNBD_SKIP = 5,
    UTDNBD_ERR = 6,
};

static int is_blank_line_vector(struct string_vector *vec)
{
    for (size_t i = 0; i < vec->size; i++)
    {
        if (!is_blank_line(vec->data[i]))
        {
            return 0;
        }
    }
    return 1;
}

static void update_special_variables(struct rule *r, struct parsed_make *make)
{
    char *targetvar_val = strdup(r->target);
    char *targetvar_key = strdup("@");
    char *firstdepvar_val = strdup(get_first_dependency_target(r));
    char *firstdepvar_key = strdup("<");
    // this one already allocates memory
    char *alldepvar_val = get_all_dependency_target(r);
    char *alldepvar_key = strdup("^");
    hash_map_insert(make->variables, targetvar_key, targetvar_val);
    hash_map_insert(make->variables, firstdepvar_key, firstdepvar_val);
    hash_map_insert(make->variables, alldepvar_key, alldepvar_val);
}

static int execute_command(char *command)
{
    int pid = fork();
    if (pid == -1)
    {
        ERR("_internal error: Failed to fork")
        return 1;
    }
    else if (pid == 0)
    {
        if (execl("/bin/sh", "sh", "-c", command, NULL) == -1)
        {
            ERR("_internal error: Failed execl call")
            return 1;
        }
    }
    else
    {
        int status;
        int child_pid = waitpid(pid, &status, 0);
        if (child_pid == -1)
        {
            ERR("_internal error: Failed to wait for child")
            return 1;
        }
        if (WEXITSTATUS(status) != 0)
        {
            fprintf(
                stderr,
                "minimake: *** command returned with non 0 code '%d'.  Stop.\n",
                WEXITSTATUS(status));
            return 1;
        }
    }
    return 0;
}

static int execute_commands(struct rule *r, struct parsed_make *make)
{
    if (r->recipe == NULL)
    {
        return 0;
    }
    update_special_variables(r, make);
    for (size_t i = 0; i < r->recipe->size; i++)
    {
        char *command = string_vector_get(r->recipe, i);
        if (command == NULL)
        {
            return 1;
        }
        size_t bytes = 0;
        int err = 0;
        struct expansion_input input = { command, make->variables, 0 };
        command = expand_variables(&input, &bytes, &err);
        if (string_vector_replace(command, i, r->recipe) != 0)
        {
            return 1;
        }
        if (err)
        {
            return 1;
        }
        if (command[0] != '@')
        {
            puts(command);
            fflush(stdout);
        }
        else
        {
            command++;
        }
        if (execute_command(command) != 0)
        {
            return 1;
        }
        fflush(stdout);
        fflush(stderr);
    }
    return 0;
}

/*
IF NO RULE AND NO FILE:
IS_NOTHING_TO_BE_DONE will return false
IS_UP_TO_DATE will say it's an error

IF NO RULE AND FILE
IS_NOTHING_TO_BE_DONE will return false
IS_UP_TO_DATE will return true
therefore nothing to be done though it should be up to date (4 values; true,
false, err, special)

IF RULE AND NO FILE
IS_NOTHING_TO_BE_DONE will return ?
IS_UP_TO_DATE will return false

IF RULE AND FILE
IS_NOTHING_TO_BE_DONE will return ?
IS_UP_TO_DATE will return ?
*/
static int is_up_to_date(char *target, struct rule *r,
                         struct rule_vector *rules);
static int is_nothing_to_be_done(struct rule *r, struct rule_vector *rules);

static int is_nothing_to_be_done(struct rule *r, struct rule_vector *rules)
{
    if (r == NULL)
    {
        return 0;
    }
    if (r->recipe != NULL && !is_blank_line_vector(r->recipe))
    {
        return 0;
    }
    for (size_t i = 0; i < r->dependencies->size; i++)
    {
        char *dep_target = r->dependencies->data[i];
        struct rule *dep = find_rule_matching_target(rules, dep_target, NULL);
        if (!is_nothing_to_be_done(dep, rules)
            && is_up_to_date(dep_target, dep, rules) != UTD_TRUE)
        {
            return 0;
        }
    }
    return 1;
}

static int is_up_to_date(char *target, struct rule *r,
                         struct rule_vector *rules)
{
    int phony = is_phony(rules, target);
    if (phony && r == NULL)
    {
        return UTD_SPECIAL;
    }
    else if (phony)
    {
        return UTD_FALSE;
    }
    char *true_target = r == NULL ? target : r->target;
    struct stat target_stat;
    if (stat(true_target, &target_stat) != 0)
    {
        if (r == NULL)
        {
            return UTD_ERR;
        }
        else
        {
            return UTD_FALSE;
        }
    }
    if (r == NULL)
    {
        return UTD_SPECIAL;
    }
    for (size_t i = 0; i < r->dependencies->size; i++)
    {
        char *dep_target = r->dependencies->data[i];
        struct stat dep_stat;
        if (stat(dep_target, &dep_stat) == 0)
        {
            // if mtime is smaller that means you are older
            if (target_stat.st_mtime <= dep_stat.st_mtime)
            {
                return UTD_FALSE;
            }
        }
        struct rule *dep = find_rule_matching_target(rules, dep_target, NULL);
        if (!is_nothing_to_be_done(dep, rules)
            && !is_up_to_date(dep_target, dep, rules))
        {
            return UTD_FALSE;
        }
    }
    return UTD_TRUE;
}

static int handle_utd_nbd(struct utd_nbd_input *input, struct parsed_make *make,
                          struct string_vector *already_built, char *needed_by)
{
    int nbd = is_nothing_to_be_done(input->r, make->rules);
    int utd = string_vector_contains(input->target, already_built)
        ? UTD_TRUE
        : is_up_to_date(input->target, input->r, make->rules);
    if (utd == UTD_ERR)
    {
        if (needed_by == NULL)
        {
            fprintf(stderr,
                    "minimake: *** No rule to make target '%s'.  Stop.\n",
                    input->target);
        }
        else
        {
            fprintf(stderr,
                    "minimake: *** No rule to make target '%s', needed by "
                    "'%s'.  Stop.\n",
                    input->target, needed_by);
        }
        return UTDNBD_ERR;
    }
    else if (nbd || utd == UTD_SPECIAL)
    {
        if (needed_by == NULL)
        {
            printf("minimake: Nothing to be done for '%s'.\n", input->target);
        }
        return UTDNBD_SKIP;
    }
    else if (utd == UTD_TRUE)
    {
        if (needed_by == NULL)
        {
            if (is_phony(make->rules, input->target))
            {
                printf("minimake: Nothing to be done for '%s'.\n",
                       input->target);
            }
            else
            {
                printf("minimake: '%s' is up to date.\n", input->target);
            }
        }
        return UTDNBD_SKIP;
    }
    return UTDNBD_BUILD;
}

static struct rule *make_temporary_target_rule(struct rule *r, char *glob)
{
    if (r == NULL)
    {
        return NULL;
    }
    struct rule *tmp = rule_init();
    if (tmp == NULL)
    {
        return NULL;
    }
    char *new_target = replace_glob(glob, r->target);
    if (new_target == NULL)
    {
        return NULL;
    }
    tmp->target = new_target;
    tmp->dependencies = string_vector_init();
    if (tmp->dependencies == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < r->dependencies->size; i++)
    {
        char *dep = r->dependencies->data[i];
        char *new_dep = replace_glob(glob, dep);
        if (new_dep == NULL)
        {
            return NULL;
        }
        if (string_vector_push_back(tmp->dependencies, new_dep) != 0)
        {
            return NULL;
        }
    }
    tmp->recipe = string_vector_init();
    if (tmp->recipe == NULL)
    {
        return NULL;
    }

    if (r->recipe == NULL)
    {
        return tmp;
    }
    for (size_t i = 0; i < r->recipe->size; i++)
    {
        char *recipe = r->recipe->data[i];
        char *recipe_dup = strdup(recipe);
        if (recipe_dup == NULL)
        {
            return NULL;
        }
        if (string_vector_push_back(tmp->recipe, recipe_dup) != 0)
        {
            return NULL;
        }
    }
    return tmp;
}

int build_target(char *target, struct parsed_make *make,
                 struct string_vector *already_built, char *needed_by)
{
    char *glob = NULL;
    struct rule *r = find_rule_matching_target(make->rules, target, &glob);
    int foundrule = r != NULL;
    r = make_temporary_target_rule(r, glob);
    if (r == NULL && foundrule)
    {
        ERR("_internal error: Failed to allocate memory");
        return 1;
    }
    if (glob != NULL)
    {
        free(glob);
    }

    struct utd_nbd_input input = { target, r };
    int utdnbd = handle_utd_nbd(&input, make, already_built, needed_by);
    if (utdnbd != UTDNBD_BUILD)
    {
        if (r != NULL)
        {
            rule_free(r);
        }
        return utdnbd == UTDNBD_ERR ? 1 : 0;
    }

    if (build_targets(r->dependencies, make, already_built, target) != 0)
    {
        rule_free(r);
        return 1;
    }

    if (execute_commands(r, make) != 0)
    {
        rule_free(r);
        return 1;
    }

    if (string_vector_push_back(already_built, target) != 0)
    {
        rule_free(r);
        ERR("_internal error: Failed to allocate memory");
        return 1;
    }

    rule_free(r);
    return 0;
}

int build_targets(struct string_vector *targets, struct parsed_make *make,
                  struct string_vector *already_built, char *needed_by)
{
    for (size_t i = 0; i < targets->size; i++)
    {
        char *target = string_vector_get(targets, i);
        if (target == NULL)
        {
            return 1;
        }
        if (build_target(target, make, already_built, needed_by) != 0)
        {
            return 1;
        }
    }
    return 0;
}
