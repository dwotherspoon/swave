#include <loader.h>

int loader_read_chunk(FILE *fptr, struct loader_chunk_t *chunk, bool is_be) {
    size_t sz_read;

    sz_read = fread(chunk->id.str, 4, 1, fptr);
    if (sz_read != 1) {
        printf("ERROR: Failed to read id of chunk.\n");
        return -1;
    }
    chunk->id.val = !is_be ? SWAP_INT32(chunk->id.val) : chunk->id.val;

    sz_read = fread(&chunk->len, 4, 1, fptr);
    if (sz_read != 1) {
        printf("ERROR: Failed to read length of chunk.\n");
        return -1;
    }
    chunk->len = is_be ? SWAP_INT32(chunk->len) : chunk->len;

    printf("Read chunk with FourCC: %.*s (%x), length: %u\n", 4, chunk->id.str, chunk->id.val, chunk->len);

    chunk->data = (uint8_t*)malloc(chunk->len);
    if (chunk->data == NULL) {
        printf("ERROR: Malloc of chunk data failed.\n");
        return -1;
    }

    sz_read = fread(chunk->data, 1, chunk->len, fptr);
    if (sz_read != chunk->len) {
        printf("ERROR: Tried to read %u bytes from chunk but read %zu\n", chunk->len, sz_read);
        return -1;
    }

    return 0;
}

int loader_process_imap(struct loader_chunk_t *chunk, struct loader_imap_t *imap, bool is_be) {
    if (chunk->id.val != FOURCC_IMAP) {
        printf("ERROR: Expected imap chunk, got %.*s\n", 4, chunk->id.str);
        return -1;
    }
    if (chunk->len != sizeof(struct loader_imap_t)) {
        printf("ERROR: Unexpected imap chunk length, expected %zu got %u\n", sizeof(struct loader_imap_t), chunk->len);
        return -1;
    }
    memcpy(imap, chunk->data, chunk->len);
    /* Swap fields if required */
    if (is_be) {
        imap->mmap_count = SWAP_INT32(imap->mmap_count);
        imap->mmap_offset = SWAP_INT32(imap->mmap_offset);
        imap->mmap_version = SWAP_INT32(imap->mmap_version);
        imap->reserved = SWAP_INT32(imap->reserved);
        imap->reserved1 = SWAP_INT32(imap->reserved1);
        imap->reserved2 = SWAP_INT32(imap->reserved2);
    }
    printf("IMAP CHUNK INFORMATION:\n"
            "\tMMAP Count:   %u\n"
            "\tMMAP Offset:  %u\n"
            "\tMMAP Version: 0x%08x\n"
            "\tReserved:     0x%08x\n"
            "\tReserved 1:   0x%08x\n"
            "\tReserved 2:   0x%08x\n",
            imap->mmap_count, imap->mmap_offset, imap->mmap_version,
            imap->reserved, imap->reserved1, imap->reserved2);
    return 0;
}

int loader_process_mmap(struct loader_chunk_t *chunk, struct loader_mmap_t *mmap, bool is_be) {
    if (chunk->id.val != FOURCC_MMAP) {
        printf("ERROR: Expected mmap chunk, got %.*s\n", 4, chunk->id.str);
        return -1;
    }
    size_t properties_sz = (void*)&mmap->entries - (void*)mmap;
    memcpy(mmap, chunk->data, properties_sz);
    if (is_be) {
        mmap->properties_sz = SWAP_INT16(mmap->properties_sz);
        mmap->resource_sz = SWAP_INT16(mmap->resource_sz);
        mmap->max_resource_count = SWAP_INT32(mmap->max_resource_count);
        mmap->used_resource_count = SWAP_INT32(mmap->used_resource_count);
        mmap->first_junk_id = SWAP_INT32(mmap->first_junk_id);
        mmap->old_mmap_id = SWAP_INT32(mmap->old_mmap_id);
        mmap->first_free_id = SWAP_INT32(mmap->first_free_id);
    }
    if (mmap->properties_sz != properties_sz) {
        printf("ERROR: Properties size mismatch, expected %zu got %u\n", properties_sz, mmap->properties_sz);
        return - 1;
    }
    /*
    if (mmap->resource_sz != sizeof(struct loader_resource_t)) {
        printf("ERROR: Resource size mismatch, expected %zu got %u\n", sizeof(struct loader_resource_t), mmap->resource_sz);
        return -1;
    } */
    printf("MMAP CHUNK INFORMATION:\n"
            "\tProperties Size:     %u\n"
            "\tResoruce Size:       %u\n"
            "\tMax Resource Count:  %u\n"
            "\tUsed Resource Count: %u\n"
            "\tFirst Junk ID:       %u\n"
            "\tOld MMAP ID:         %u\n"
            "\tFirst Free ID:       %u\n",
            mmap->properties_sz, mmap->resource_sz, mmap->max_resource_count,
            mmap->used_resource_count, mmap->first_junk_id, mmap->old_mmap_id,
            mmap->first_free_id);

    return 0;
}

int loader_load_file(char *fname) {
    union fourcc_t tmp;
    size_t sz_read;
    uint32_t len;
    bool is_be;
    struct loader_chunk_t cur_chunk;
    struct loader_imap_t imap;
    struct loader_mmap_t mmap;


    FILE *fptr = fopen(fname, "r");

    if (fptr == NULL) {
        printf("ERROR: Failed to open \"%s\".\n", fname);
        return -1;
    }

    /* Start by reading the FOURCC code */
    sz_read = fread(tmp.str, 4, 1, fptr);
    printf("FourCC: %.*s (%x)\n", 4, tmp.str, tmp.val);

    /* This detection isn't safe for BE systems */
    is_be = tmp.val == FOURCC_RIFX;
    printf(is_be ? "Big-endian file.\n" : "Little-endian file.\n");

    sz_read = fread(&len, 4, 1, fptr);
    len = is_be ? SWAP_INT32(len) : len;

    /* Read next FOURCC code (codec) */
    sz_read = fread(tmp.str, 4, 1, fptr);
    tmp.val = !is_be ? SWAP_INT32(tmp.val) : tmp.val;
    printf("Type FourCC: %.*s (%x), length of file: %u\n", 4, tmp.str, tmp.val, len);

    if (tmp.val != FOURCC_MV93 && tmp.val != FOURCC_MC95) {
        printf("ERROR: Unsupported codec (afterburner).\n");
        return -1;
    }

    /* Read and process imap chunk */
    if (loader_read_chunk(fptr, &cur_chunk, is_be) != 0) {
        printf("ERROR: Failed to read imap chunk.\n");
        return -1;
    }
    if (loader_process_imap(&cur_chunk, &imap, is_be) != 0) {
        printf("ERROR: Failed to process imap chunk.\n");
        return -1;
    }
    /* Free the loaded data */
    free(cur_chunk.data);

    /* Move to mmap offset */
    fseek(fptr, imap.mmap_offset, SEEK_SET);

    /* Read and process mmap chunk */
    if (loader_read_chunk(fptr, &cur_chunk, is_be) != 0) {
        printf("ERROR: Failed to read mmap chunk.\n");
        return -1;
    }
    if (loader_process_mmap(&cur_chunk, &mmap, is_be) != 0) {
        printf("ERROR: Failed to process mmap chunk.\n");
        return -1;
    }
    /* Free the loaded data */
    free(cur_chunk.data);


    fclose(fptr);
    return 0;
}
