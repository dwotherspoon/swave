#ifndef _KEYS_H
#define _KEYS_H

#include <director/util.h>

typedef struct __attribute__((__packed__)) {
    int32_t resource_id;
    int32_t owner_id;
    fourcc_t type;
} keys_entry_t;

typedef struct __attribute__((__packed__)) {
    uint32_t length;
    uint16_t properties_length;
    uint16_t entry_length;
    int32_t max_entry_count;
    int32_t used_entry_count;
    keys_entry_t *entries;
} keys_t;

int keys_process_chunk(director_chunk_t *chunk, keys_t *keys, endian_t endianness);
void keys_print(keys_t *keys);
void keys_print_entry(keys_entry_t *entry);
void keys_print_entries(keys_t *keys);

#endif
