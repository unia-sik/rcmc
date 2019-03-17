#pragma once

#include "share.h"
#include <stdint.h>

typedef struct string_map_item {
    uint64_t value;
    bool valid;
    struct string_map_item* next[256];
} string_map_item_t;

typedef struct string_map {
    string_map_item_t root;
    uint64_t maxKeylen;
} string_map_t;

void string_map_init(string_map_t* map);
void string_map_free(string_map_t* map);

void string_map_insert(string_map_t* map, const char* key, const uint64_t value);
bool string_map_get(string_map_t* map, const char* key, uint64_t* value);
void string_map_remove(string_map_t* map, const char* key);

void string_map_print(string_map_t* map);
