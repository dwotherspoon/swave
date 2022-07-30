#ifndef _MMAP_H
#define _MMAP_H

#include <stdint.h>

#include <director/util.h>

typedef struct __attribute__((__packed__)) {
	fourcc_t type;
	int32_t length;
	int32_t offset;
    uint16_t flags;
    uint16_t reserved;
    int32_t next_id;
} mmap_entry_t;

typedef struct __attribute__((__packed__)) {
    int32_t entry_id;
    int32_t offset;
    int32_t compressed_length;
    int32_t uncompressed_length;
    fourcc_t compression_type;
    fourcc_t type;
} abmp_entry_t;

typedef union {
    mmap_entry_t *mmap;
    abmp_entry_t *abmp;
} mmap_entries_t;

typedef struct __attribute__((__packed__)) {
    uint32_t length;
    uint16_t properties_length;
    uint16_t entry_length;
    int32_t max_entry_count;
    int32_t used_entry_count;
    int32_t first_junk_id;
    int32_t old_mmap_id;
    int32_t first_free_id;
    mmap_entry_t *entries;
} mmap_t;

int mmap_process_chunk(director_chunk_t *chunk, mmap_t *mmap, endian_t endianness);
int32_t mmap_find_entry(mmap_t *mmap, fourcc_t type, int32_t start);
void mmap_print(mmap_t *mmap);
void mmap_print_entry(mmap_entry_t *entry);
void mmap_print_entries(mmap_t *mmap);

#endif
