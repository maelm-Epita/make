#ifndef RULE_VECTOR_H
#define RULE_VECTOR_H

#include <stdlib.h>

#include "rule.h"

struct rule_vector
{
    struct rule **data;
    size_t size;
    size_t capacity;
};

static inline struct rule_vector *rule_vector_init(void)
{
    struct rule_vector *vector = malloc(sizeof(struct rule_vector));
    if (vector == (NULL))
    {
        return (NULL);
    }
    vector->capacity = 0;
    vector->size = 0;
    vector->data = (NULL);
    return vector;
}
static inline void rule_vector_free(struct rule_vector *vector)
{
    if (vector == (NULL))
    {
        return;
    }
    {
        if (vector->data != (NULL))
        {
            for (size_t i = 0; i < vector->size; i++)
            {
                struct rule *r = vector->data[i];
                rule_free(r);
            }
            free(vector->data);
        }
        free(vector);
    }
}
static inline size_t rule_vector_size(const struct rule_vector *vector)
{
    if (vector == (NULL))
    {
        return 0;
    }
    return vector->size;
}
static inline int rule_vector_insert(struct rule_vector *vector, size_t pos,
                                     struct rule *element)
{
    if (vector == (NULL))
    {
        return 1;
    }
    if (pos > vector->size)
    {
        return 1;
    }
    if (vector->size == vector->capacity)
    {
        size_t new_capacity;
        if (vector->capacity == 0)
        {
            new_capacity = 1;
        }
        else
        {
            new_capacity = vector->capacity * 2;
        }
        vector->capacity = new_capacity;
        vector->data =
            realloc(vector->data, sizeof(struct rule *) * vector->capacity);
        if (vector->data == (NULL) && new_capacity != 0)
        {
            return 1;
        }
    }
    for (size_t j = vector->size; j > pos; j--)
    {
        vector->data[j] = vector->data[j - 1];
    }
    vector->data[pos] = element;
    vector->size++;
    return 0;
}
static inline int rule_vector_push_back(struct rule_vector *vector,
                                        struct rule *element)
{
    if (vector == (NULL))
    {
        return 1;
    };
    return rule_vector_insert(vector, vector->size, element);
}
static inline struct rule *rule_vector_get(struct rule_vector *vector,
                                           size_t pos)
{
    if (vector == (NULL))
    {
        return 0;
    };
    if (pos >= vector->size)
    {
        return 0;
    }
    return vector->data[pos];
}

#endif /* ! RULE_VECTOR_H */
