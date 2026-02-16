#ifndef EXPANSION_H
#define EXPANSION_H

#include "hash_map.h"

struct expansion_input
{
    char *line;
    struct hash_map *variables;
    int variable_definition;
};

struct span
{
    size_t start;
    size_t end;
};

char *expand_variables(struct expansion_input *input, size_t *out_bytes,
                       int *err);

#endif /* ! EXPANSION_H */
