#include "string_map.h"

#include <stdlib.h>
#include <string.h>

void string_map_free_item(string_map_item_t* item) {
    if (item != NULL) {
        for (int i = 0; i < 256; i++) {            
            string_map_free_item(item->next[i]);
        }
        free(item);
    }
}

void string_map_init(string_map_t* map)
{
    memset(&map->root, 0, sizeof(string_map_t));
    map->maxKeylen = 0;
}

void string_map_free(string_map_t* map)
{
    for (int i = 0; i < 256; i++) {            
        string_map_free_item(map->root.next[i]);
    }   
    memset(&map->root, 0, sizeof(string_map_t));
}


string_map_item_t* string_map_get_item(string_map_t* map, const char* key, bool force) {
    string_map_item_t* current = &map->root;
    for (int i = 0; key[i] != '\0'; i++) {
        if (current->next[(unsigned)key[i]] == NULL) {
            if (force) {
                current->next[(unsigned)key[i]] = calloc(1, sizeof(string_map_item_t));
            } else {
                return NULL;
            }
        }
        
        current = current->next[(unsigned)key[i]];
        
        if (map->maxKeylen < (i + 1)) {
            map->maxKeylen = i + 1;
        }
    }   
    
    return current;
}

void string_map_insert(string_map_t* map, const char* key, const uint64_t value)
{
    string_map_item_t* result = string_map_get_item(map, key, true);
    
    result->value = value;
    result->valid = true;
}

bool string_map_get(string_map_t* map, const char* key, uint64_t* value)
{
    string_map_item_t* result = string_map_get_item(map, key, false);
    
    if (result == NULL || !result->valid) {
        return false;
    }
    
    *value = result->value;    
    return true;
}

void string_map_remove(string_map_t* map, const char* key)
{
    int len = strlen(key);
    char last = key[len - 1];
    char shortKey[len];
    strncpy(shortKey, key, len);
    shortKey[len - 1] = '\0';
    
    string_map_item_t* result = string_map_get_item(map, shortKey, false);
    
    if (result == NULL || result->next[(unsigned int)last] == NULL) {
        return;
    }
    result->next[(unsigned int)last]->valid = false;
    
    for (int i = 0; i < 256; i++) {
        if (result->next[(unsigned int)last]->next[i] != NULL) {
            return;
        }
    }
    
    free(result->next[(unsigned int)last]);
    result->next[(unsigned int)last] = NULL;
}

void string_map_print_item(string_map_item_t* item, char* outputMsg, uint64_t current, uint64_t maxKeylen)
{
    if (item->valid) {
        printf("%*s| %ld\n",(int)maxKeylen, outputMsg, item->value);
    }
    
    for (int i = 0; i < 256; i++) {
        outputMsg[current] = (char)i;
        if (item->next[i] != NULL) {
            string_map_print_item(item->next[i], outputMsg, current + 1, maxKeylen);
        }
    }
    
    outputMsg[current] = '\0';
}

void string_map_print(string_map_t* map)
{
    char outputMsg[map->maxKeylen + 1];
    memset(outputMsg, 0, map->maxKeylen + 1);    
    string_map_print_item(&map->root, outputMsg, 0, map->maxKeylen);
}
