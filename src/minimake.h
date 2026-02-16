#ifndef MINIMAKE_H
#define MINIMAKE_H

#include "string_vector.h"

struct options
{
    int display_help;
    int pretty_print;
    struct string_vector *files;
    struct string_vector *targets;
};

int minimake(struct options opts);

#endif /* ! MINIMAKE_H */
