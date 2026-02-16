#include <string.h>
#include <sys/stat.h>

#include "codes.h"
#include "minimake.h"

#define FREE                                                                   \
    {                                                                          \
        if (opts.targets != NULL)                                              \
        {                                                                      \
            if (opts.targets->data != NULL)                                    \
            {                                                                  \
                free(opts.targets->data);                                      \
            }                                                                  \
            free(opts.targets);                                                \
        }                                                                      \
        if (opts.files != NULL)                                                \
        {                                                                      \
            if (opts.files->data != NULL)                                      \
            {                                                                  \
                free(opts.files->data);                                        \
            }                                                                  \
            free(opts.files);                                                  \
        }                                                                      \
    }

enum OPT_TYPE
{
    OPT_NONE,
    OPT_FILE,
};

static int parse_arguments(int argc, char **argv, struct options *out)
{
    enum OPT_TYPE option_type = OPT_NONE;
    out->targets = string_vector_init();
    out->files = string_vector_init();
    for (int i = 0; i < argc; i++)
    {
        char *argument = argv[i];
        if (option_type == OPT_NONE)
        {
            if (argument[0] == 0)
            {
                ERR("empty string invalid as argument");
                return 1;
            }
            else if (strcmp(argument, "-h") == 0)
            {
                out->display_help = 1;
                return 0;
            }
            else if (strcmp(argument, "-p") == 0)
            {
                out->pretty_print = 1;
            }
            else if (strcmp(argument, "-f") == 0)
            {
                if (i == argc - 1)
                {
                    ERR("no filename provided");
                    return 1;
                }
                option_type = OPT_FILE;
            }
            else
            {
                if (string_vector_push_back(out->targets, argument) != 0)
                {
                    ERR("_internal error: Failed to allocate memory");
                    return 1;
                }
            }
        }
        else
        {
            switch (option_type)
            {
            case OPT_FILE: {
                struct stat arg_stat = { 0 };
                if (stat(argument, &arg_stat) != 0)
                {
                    i--;
                }
                else if (string_vector_push_back(out->files, argument) != 0)
                {
                    ERR("_internal error: Failed to allocate memory");
                    return 1;
                }
                break;
            }
            case OPT_NONE:
                return 1;
            }
            option_type = OPT_NONE;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct options opts = { 0 };
    if (parse_arguments(argc - 1, &argv[1], &opts) != 0)
    {
        FREE;
        return CODE_FAILURE;
    }
    int code = minimake(opts);
    FREE;
    return code;
}
