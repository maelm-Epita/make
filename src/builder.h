#ifndef BUILDER_H
#define BUILDER_H

#include "parser.h"
#include "string_vector.h"

struct utd_nbd_input
{
    char *target;
    struct rule *r;
};

int build_target(char *target, struct parsed_make *make,
                 struct string_vector *already_built, char *needed_by);

int build_targets(struct string_vector *targets, struct parsed_make *make,
                  struct string_vector *already_built, char *needed_by);

#endif /* ! BUILDER_H */
