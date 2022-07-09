#ifndef _IMAP_H
#define _IMAP_H

#include <director/util.h>

typedef struct __attribute__((__packed__)) {
    uint32_t length;
    uint32_t mmap_count;
    uint32_t mmap_offset;
    uint32_t mmap_version;
    uint32_t reserved;
    uint32_t reserved1;
    uint32_t reserved2;
} imap_t;

int imap_process_chunk(director_chunk_t *chunk, imap_t *imap, endian_t endianness);
void imap_print(imap_t *imap);

#endif
