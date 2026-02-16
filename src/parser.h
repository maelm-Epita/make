#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "hash_map.h"
#include "rule.h"

struct parsed_make
{
    struct rule_vector *rules;
    struct hash_map *variables;
};

int parse(FILE *makefile, struct parsed_make *out);

#endif /* ! PARSER_H */
