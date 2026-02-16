#ifndef STRING_VECTOR_H
#define STRING_VECTOR_H

#include <stdlib.h>
#include <string.h>

struct string_vector
{
    char **data;
    size_t size;
    size_t capacity;
};

static inline struct string_vector *string_vector_init(void)
{
    struct string_vector *vector = malloc(sizeof(struct string_vector));
    if (vector == (NULL))
    {
        return (NULL);
    }
    vector->capacity = 0;
    vector->size = 0;
    vector->data = (NULL);
    return vector;
}

static inline void string_vector_free(struct string_vector *vector)
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
                free(vector->data[i]);
            }
            free(vector->data);
        }
        free(vector);
    }
}

static inline size_t string_vector_size(const struct string_vector *vector)
{
    if (vector == (NULL))
    {
        return 0;
    }
    return vector->size;
}

static inline int string_vector_insert(struct string_vector *vector, size_t pos,
                                       char *element)
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
        vector->data = realloc(vector->data, sizeof(char *) * vector->capacity);
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

static inline int string_vector_push_back(struct string_vector *vector,
                                          char *element)
{
    if (vector == (NULL))
    {
        return 1;
    };
    return string_vector_insert(vector, vector->size, element);
}

static inline char *string_vector_get(struct string_vector *vector, size_t pos)
{
    if (vector == (NULL))
    {
        return NULL;
    };
    if (pos >= vector->size)
    {
        return NULL;
    }
    return vector->data[pos];
}

static inline int string_vector_replace(char *r, size_t pos,
                                        struct string_vector *vec)
{
    if (vec == NULL || pos > vec->size)
    {
        return 1;
    }
    vec->data[pos] = r;
    return 0;
}

static inline int string_vector_contains(char *str, struct string_vector *vec)
{
    if (vec == NULL)
    {
        return 0;
    }
    for (size_t i = 0; i < vec->size; i++)
    {
        char *el = string_vector_get(vec, i);
        if (el == NULL)
        {
            return 0;
        }
        if (strcmp(str, el) == 0)
        {
            return 1;
        }
    }
    return 0;
}

#endif /* ! STRING_VECTOR_H */
