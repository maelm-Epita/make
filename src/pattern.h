#ifndef PATTERN_H
#define PATTERN_H

#include <stddef.h>

struct pattern_helpers
{
    int foundglob;
    size_t glob_i;
};

int satisfies_pattern(char *str, char *pattern, char **out_glob);
char *replace_glob(char *glob, char *pattern);

#endif /* ! PATTERN_H */
