#include "hash_map.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -- AUX -- */
static struct pair_list *find_key(struct pair_list *list, char *key)
{
    while (list != NULL && strcmp(list->key, key) != 0)
    {
        list = list->next;
    }
    return list;
}

static void list_destroy(struct pair_list *list)
{
    if (list == NULL)
    {
        return;
    }
    if (list->key != NULL)
    {
        free(list->key);
    }
    if (list->value != NULL)
    {
        free(list->value);
    }
    list_destroy(list->next);
    free(list);
}

static void list_print(struct pair_list *list)
{
    if (list == NULL)
    {
        return;
    }
    while (list != NULL)
    {
        printf("%s: %s", list->key, list->value);
        if (list->next != NULL)
        {
            printf(", ");
        }
        list = list->next;
    }
    printf("\n");
}

static size_t hash(const char *key)
{
    if (!key)
        return 0;

    uint32_t hash = 2166136261; // FNV offset basis
    uint32_t prime = 16777619; // FNV prime

    while (*key)
    {
        hash ^= *key;
        hash *= prime;
        key++;
    }

    return hash;
}

static void hash_map_rehash(struct hash_map *hash_map, size_t old_capacity)
{
    for (size_t i = 0; i < old_capacity; i++)
    {
        struct pair_list *pair_list = hash_map->data[i];
        if (pair_list == NULL)
        {
            continue;
        }
        hash_map->data[i] = 0;
        hash_map->element_count--;
        struct pair_list *element = pair_list;
        while (element != NULL)
        {
            hash_map_insert(hash_map, element->key, element->value);
            element = element->next;
        }
        list_destroy(pair_list);
    }
}

static int hash_map_resize(struct hash_map *hash_map, size_t new_capacity)
{
    size_t old_capacity = hash_map->capacity;
    hash_map->capacity = new_capacity;
    hash_map->data = realloc(hash_map->data,
                             sizeof(struct pair_list *) * hash_map->capacity);
    for (size_t i = old_capacity; i < hash_map->capacity; i++)
    {
        hash_map->data[i] = 0;
    }
    if (hash_map->data == NULL && new_capacity != 0)
    {
        return 0;
    }
    hash_map_rehash(hash_map, old_capacity);
    return 1;
}
/* ! -- AUX -- */

struct hash_map *hash_map_init(void)
{
    struct hash_map *map = malloc(sizeof(struct hash_map));
    if (map == NULL)
    {
        return NULL;
    }
    map->capacity = DEFAULT_CAPACITY;
    map->element_count = 0;
    map->data = calloc(map->capacity, sizeof(struct pair_list));
    map->insert_order_head = NULL;
    map->insert_order_tail = NULL;
    return map;
}

int hash_map_insert(struct hash_map *hash_map, char *key, char *value)
{
    if (hash_map == NULL || key == NULL || value == NULL)
    {
        return 1;
    }
    if (hash_map->element_count >= hash_map->capacity / 2)
    {
        size_t new_capacity =
            hash_map->capacity == 0 ? 1 : hash_map->capacity * 2;
        hash_map_resize(hash_map, new_capacity);
    }
    size_t hash_index = hash(key) % hash_map->capacity;
    struct pair_list *pair_list = hash_map->data[hash_index];
    struct pair_list *list = find_key(pair_list, key);
    // if only needs updating
    if (pair_list != NULL && list != NULL)
    {
        free(list->value);
        free(key);
        list->value = value;
    }
    else
    {
        struct pair_list *el = malloc(sizeof(struct pair_list));
        if (el == NULL)
        {
            return 1;
        }
        el->key = key;
        el->value = value;
        el->next = pair_list;
        el->insert_order_next = NULL;
        hash_map->data[hash_index] = el;
        if (hash_map->element_count == 0)
        {
            hash_map->insert_order_head = el;
        }
        if (hash_map->insert_order_tail != NULL)
        {
            hash_map->insert_order_tail->insert_order_next = el;
        }
        hash_map->insert_order_tail = el;
        // if pair_list was null then we created a new list, else we prepended
        if (pair_list == NULL)
        {
            hash_map->element_count++;
        }
    }
    return 0;
}

const char *hash_map_get(struct hash_map *hash_map, char *key)
{
    if (hash_map == NULL)
    {
        return NULL;
    }
    size_t hash_index = hash(key) % hash_map->capacity;
    struct pair_list *list = find_key(hash_map->data[hash_index], key);
    if (list == NULL)
    {
        return NULL;
    }
    return list->value;
}

void hash_map_free(struct hash_map *hash_map)
{
    if (hash_map == NULL)
    {
        return;
    }
    for (size_t i = 0; i < hash_map->capacity; i++)
    {
        list_destroy(hash_map->data[i]);
    }
    free(hash_map->data);
    free(hash_map);
}
void hash_map_dump(struct hash_map *hash_map)
{
    size_t size = hash_map->capacity;
    for (size_t i = 0; i < size; i++)
    {
        list_print(hash_map->data[i]);
    }
}
