#include <director/imap.h>

#include <stdio.h>
#include <string.h>

int imap_process_chunk(director_chunk_t *chunk, imap_t *imap, endian_t endianness) {
    if (chunk->id.val != FOURCC_IMAP) {
        printf("ERROR: Expected imap chunk, got %.*s\n", 4, chunk->id.str);
        return -1;
    }

    imap->id = chunk->id;
    imap->length = chunk->length;
    /*
    if (chunk->length != ((uint8_t*)&imap->mmap_count - (uint8_t*)imap)) {
        printf("ERROR: Unexpected imap chunk length, expected %zu got %u\n", sizeof(imap_t), chunk->length);
        return -1;
    }*/
    memcpy(&imap->mmap_count, chunk->data, chunk->length);
    /* Swap fields if required */
    if (endianness == BIG_ENDIAN) {
        imap->mmap_count = SWAP_INT32(imap->mmap_count);
        imap->mmap_offset = SWAP_INT32(imap->mmap_offset);
        imap->mmap_version = SWAP_INT32(imap->mmap_version);
        imap->reserved = SWAP_INT32(imap->reserved);
        imap->reserved1 = SWAP_INT32(imap->reserved1);
        imap->reserved2 = SWAP_INT32(imap->reserved2);
    }

    return 0;
}

void imap_print(imap_t *imap) {
    printf("IMAP CHUNK INFORMATION:\n"
            "\tMMAP Count:   %u\n"
            "\tMMAP Offset:  %u\n"
            "\tMMAP Version: 0x%08x\n"
            "\tReserved:     0x%08x\n"
            "\tReserved 1:   0x%08x\n"
            "\tReserved 2:   0x%08x\n",
            imap->mmap_count, imap->mmap_offset, imap->mmap_version,
            imap->reserved, imap->reserved1, imap->reserved2);
}
