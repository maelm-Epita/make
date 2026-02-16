#define _GNU_SOURCE

#include "minimake.h"

#include "builder.h"
#include "codes.h"
#include "parser.h"
#include "rule_vector.h"
#include "sys/stat.h"

#define HELP_TEXT                                                              \
    "\
Usage: minimake [options] [target] ...\n\
Options:\n\
  -h                          Print this message and exit.\n\
  -f FILE                     Read FILE as a makefile.\n\
  -p                          Don't actually run any recipe; just print them.\n\
    "

#define FREE_PARSED                                                            \
    {                                                                          \
        if (parsed.variables != NULL)                                          \
        {                                                                      \
            hash_map_free(parsed.variables);                                   \
        }                                                                      \
        if (parsed.rules != NULL)                                              \
        {                                                                      \
            rule_vector_free(parsed.rules);                                    \
        }                                                                      \
    }
#define FREE_STATIC_STRVEC                                                     \
    {                                                                          \
        if (built_targets != NULL)                                             \
        {                                                                      \
            free(built_targets->data);                                         \
            free(built_targets);                                               \
        }                                                                      \
    }

static inline void print_makefile(struct parsed_make make)
{
    struct hash_map *variables = make.variables;
    struct rule_vector *rules = make.rules;
    printf("# variables\n");
    struct pair_list *el = variables->insert_order_head;
    while (el != NULL)
    {
        printf("'%s' = '%s'\n", el->key, el->value);
        el = el->insert_order_next;
    }
    printf("# rules\n");
    for (size_t i = 0; i < rules->size; i++)
    {
        struct rule *r = rules->data[i];
        printf("(%s):", r->target);
        for (size_t j = 0; j < r->dependencies->size; j++)
        {
            printf(" [%s]", r->dependencies->data[j]);
        }
        putchar('\n');
        if (r->recipe != NULL)
        {
            for (size_t j = 0; j < r->recipe->size; j++)
            {
                printf("\t'%s'\n", r->recipe->data[j]);
            }
        }
    }
}

static inline int minimake_help(void)
{
    puts(HELP_TEXT);
    return CODE_SUCCESS;
}

static inline int minimake_print(FILE *file)
{
    struct parsed_make parsed = { 0 };
    if (parse(file, &parsed) != 0)
    {
        FREE_PARSED;
        return CODE_FAILURE;
    }
    print_makefile(parsed);
    FREE_PARSED;
    return CODE_SUCCESS;
}

static inline int minimake_make(FILE *file, struct string_vector *rules)
{
    struct string_vector *built_targets = string_vector_init();
    if (built_targets == NULL)
    {
        ERR("_internal error: Failed to allocate memory");
        return CODE_FAILURE;
    }
    int code = CODE_SUCCESS;
    struct parsed_make parsed = { 0 };
    if (parse(file, &parsed) != 0)
    {
        code = CODE_FAILURE;
    }
    if (code == CODE_SUCCESS)
    {
        if (rules->size == 0)
        {
            struct rule *first_rule = get_first_standard_target(parsed.rules);
            if (first_rule == NULL)
            {
                ERR("No targets");
                code = CODE_FAILURE;
            }
            if (code == CODE_SUCCESS
                && build_target(first_rule->target, &parsed, built_targets,
                                NULL)
                    != 0)
            {
                code = CODE_FAILURE;
            }
        }
        else
        {
            if (build_targets(rules, &parsed, built_targets, NULL) != 0)
            {
                code = CODE_FAILURE;
            }
        }
    }
    FREE_STATIC_STRVEC;
    FREE_PARSED
    return code;
}

static int minimake_get_files(struct options opts)
{
    char *filename = NULL;
    struct stat fstat;
    if (stat("makefile", &fstat) == 0)
    {
        filename = "makefile";
    }
    else if (stat("Makefile", &fstat) == 0)
    {
        filename = "Makefile";
    }
    if (filename != NULL)
    {
        if (string_vector_push_back(opts.files, filename) != 0)
        {
            ERR("_internal error: Failed to allocate memory");
            return CODE_FAILURE;
        }
    }
    else
    {
        if (opts.targets->size == 0)
        {
            ERR("No targets specified and no makefile found");
            return CODE_FAILURE;
        }
    }
    return CODE_SUCCESS;
}

static int do_minimake(struct options opts, FILE *file)
{
    if (opts.pretty_print)
    {
        return minimake_print(file);
    }
    else
    {
        return minimake_make(file, opts.targets);
    }
}

int minimake(struct options opts)
{
    if (opts.display_help)
    {
        return minimake_help();
    }
    if (opts.files->size == 0)
    {
        if (minimake_get_files(opts) != CODE_SUCCESS)
        {
            return CODE_FAILURE;
        }
    }
    if (opts.files->size == 0)
    {
        char empty_makefile[1] = { 0 };
        FILE *file = fmemopen(empty_makefile, 1, "r");
        return do_minimake(opts, file);
    }
    for (size_t i = 0; i < opts.files->size; i++)
    {
        char *filename = opts.files->data[i];
        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            fprintf(stderr, "minimake: *** No such file \'%s\'.  Stop.\n",
                    filename);
            return CODE_FAILURE;
        }
        if (do_minimake(opts, file) != CODE_SUCCESS)
        {
            return CODE_FAILURE;
        }
    }
    return CODE_SUCCESS;
}
