#define _GNU_SOURCE

#include "../src/hash_map.h"
#include "../src/string_vector.h"
#include "criterion/criterion.h"

TestSuite(dynamic_hash_map_tests);
TestSuite(vector_tests);

Test(dynamic_hash_map_tests, hash_map_init_test)
{
    struct hash_map *map = hash_map_init();
    cr_assert_eq(map->capacity, DEFAULT_CAPACITY);
    cr_assert_eq(map->element_count, 0);
    cr_assert_eq(map->insert_order_head, NULL);
    cr_assert_eq(map->insert_order_tail, NULL);
    hash_map_free(map);
}

Test(dynamic_hash_map_tests, hash_map_tests)
{
    struct hash_map *map = hash_map_init();
    char *k1 = strdup("var1");
    char *v1 = strdup("val1");
    char *k2 = strdup("var2");
    char *v2 = strdup("val2");
    char *k3 = strdup("var3");
    char *v3 = strdup("val3");
    hash_map_insert(map, k1, v1);
    cr_assert(map->element_count == 1);
    cr_assert(strcmp(v1, hash_map_get(map, k1)) == 0);
    hash_map_insert(map, k2, v2);
    cr_assert(map->element_count == 2);
    cr_assert(strcmp(v2, hash_map_get(map, k2)) == 0);
    hash_map_insert(map, k3, v3);
    cr_assert(map->element_count == 3);
    cr_assert(strcmp(v3, hash_map_get(map, k3)) == 0);
    hash_map_free(map);
}

Test(vector_tests, vector_init_test)
{
    struct string_vector *vec = string_vector_init();
    cr_assert_eq(vec->capacity, 0);
    cr_assert_eq(vec->size, 0);
    cr_assert_eq(vec->data, NULL);
}

Test(vector_tests, vector_size_test)
{
    struct string_vector *vec = string_vector_init();
    vec->size = 3;
    cr_assert_eq(string_vector_size(vec), 3);
}

Test(vector_tests, vector_insert_test)
{
    struct string_vector *vec = string_vector_init();
    char *v1 = strdup("1");
    char *v2 = strdup("2");
    char *v3 = strdup("3");
    char *v4 = strdup("4");
    string_vector_insert(vec, 0, v1);
    string_vector_insert(vec, 0, v2);
    string_vector_insert(vec, 2, v3);
    string_vector_insert(vec, 1, v4);
    cr_assert_eq(vec->data[0], v2);
    cr_assert_eq(vec->data[1], v4);
    cr_assert_eq(vec->data[2], v1);
    cr_assert_eq(vec->data[3], v3);
}

Test(vector_tests, vector_push_back_test)
{
    struct string_vector *vec = string_vector_init();
    char *v1 = strdup("1");
    char *v2 = strdup("2");
    char *v3 = strdup("3");
    char *v4 = strdup("4");
    string_vector_push_back(vec, v1);
    string_vector_push_back(vec, v2);
    string_vector_push_back(vec, v3);
    string_vector_push_back(vec, v4);
    cr_assert_eq(vec->data[0], v1);
    cr_assert_eq(vec->data[1], v2);
    cr_assert_eq(vec->data[2], v3);
    cr_assert_eq(vec->data[3], v4);
}

Test(vector_tests, vector_get_test)
{
    struct string_vector *vec = string_vector_init();
    char *v1 = strdup("1");
    char *v2 = strdup("2");
    char *v3 = strdup("3");
    char *v4 = strdup("4");
    string_vector_push_back(vec, v1);
    string_vector_push_back(vec, v2);
    string_vector_push_back(vec, v3);
    string_vector_push_back(vec, v4);
    cr_assert_eq(v3, string_vector_get(vec, 2));
}

Test(vector_tests, vector_replace_test)
{
    struct string_vector *vec = string_vector_init();
    char *v1 = strdup("1");
    char *v2 = strdup("2");
    char *v3 = strdup("3");
    char *v4 = strdup("4");
    string_vector_push_back(vec, v1);
    string_vector_push_back(vec, v2);
    string_vector_push_back(vec, v3);
    string_vector_push_back(vec, v4);
    string_vector_replace(v1, 3, vec);
    cr_assert_eq(v1, string_vector_get(vec, 3));
}

Test(vector_tests, vector_contains_test)
{
    struct string_vector *vec = string_vector_init();
    char *v1 = strdup("1");
    char *v2 = strdup("2");
    char *v3 = strdup("3");
    char *v4 = strdup("4");
    string_vector_push_back(vec, v1);
    string_vector_push_back(vec, v2);
    string_vector_push_back(vec, v3);
    string_vector_push_back(vec, v4);
    cr_assert(string_vector_contains(v1, vec));
    cr_assert(string_vector_contains(v2, vec));
    cr_assert(string_vector_contains(v3, vec));
    cr_assert(string_vector_contains(v4, vec));
}
