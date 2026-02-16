#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define DEFAULT_CAPACITY 64

struct pair_list
{
    char *key;
    char *value;
    struct pair_list *next;
    struct pair_list *insert_order_next;
};

struct hash_map
{
    struct pair_list **data;
    size_t element_count;
    size_t capacity;
    struct pair_list *insert_order_head;
    struct pair_list *insert_order_tail;
};

struct hash_map *hash_map_init(void);
int hash_map_insert(struct hash_map *hash_map, char *key, char *value);
const char *hash_map_get(struct hash_map *hash_map, char *key);
void hash_map_free(struct hash_map *hash_map);
void hash_map_dump(struct hash_map *hash_map);

#endif /* ! HASH_MAP_H */
