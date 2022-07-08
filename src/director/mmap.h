#ifndef _MMAP_H
#define _MMAP_H

#include <stdint.h>

#include <director/util.h>

typedef struct {
	fourcc_t type;
	int32_t length;
	int32_t offset;
    uint16_t flags;
    uint16_t reserved;
    int32_t next_id;
} mmap_entry_t;


typedef struct {
    fourcc_t id;
    uint32_t length;
    uint16_t properties_length;
    uint16_t entry_length;
    int32_t max_resource_count;
    int32_t used_resource_count;
    int32_t first_junk_id;
    int32_t old_mmap_id;
    int32_t first_free_id;
    mmap_entry_t *entries;
} mmap_t;

int mmap_process_chunk(director_chunk_t *chunk, mmap_t *mmap, endian_t endianness);
void mmap_print(mmap_t *mmap);
void mmap_print_entry(mmap_entry_t *entry);
void mmap_print_entries(mmap_t *mmap);

#endif