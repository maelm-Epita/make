#ifndef RULE_H
#define RULE_H

#include "string_vector.h"

struct rule
{
    char *target;
    struct string_vector *dependencies;
    struct string_vector *recipe;
};

struct rule_vector;

void rule_free(struct rule *r);
char *get_first_dependency_target(struct rule *r);
char *get_all_dependency_target(struct rule *r);
struct rule *get_first_standard_target(struct rule_vector *rules);
struct rule *find_rule_with_target(struct rule_vector *rules, char *target);
struct rule *find_rule_matching_target(struct rule_vector *rules, char *target,
                                       char **out_glob);
int is_phony(struct rule_vector *rules, char *target);
struct rule *rule_init(void);

#endif /* ! RULE_H */
