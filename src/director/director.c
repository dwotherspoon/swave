#include <director/director.h>

#include <stdlib.h>

int director_load_chunk(director_chunk_t *chunk, director_t *director, uint32_t offset) {
    size_t sz_read;
    fseek(director->fptr, offset, SEEK_SET);

    sz_read = fread(chunk->id.str, 4, 1, director->fptr);
    if (sz_read != 1) {
        printf("ERROR: Failed to read id of chunk.\n");
        return -1;
    }
    chunk->id.val = director->endianess == LITTLE_ENDIAN ? SWAP_INT32(chunk->id.val) : chunk->id.val;

    sz_read = fread(&chunk->length, 4, 1, director->fptr);
    if (sz_read != 1) {
        printf("ERROR: Failed to read length of chunk.\n");
        return -1;
    }
    chunk->length = director->endianess == BIG_ENDIAN ? SWAP_INT32(chunk->length) : chunk->length;

    printf("Read chunk with FourCC: %.*s (%x), length: %u\n", 4, chunk->id.str, chunk->id.val, chunk->length);

    chunk->data = (uint8_t*)malloc(chunk->length);
    if (chunk->data == NULL) {
        printf("ERROR: Malloc of chunk data failed.\n");
        return -1;
    }

    sz_read = fread(chunk->data, 1, chunk->length, director->fptr);
    if (sz_read != chunk->length) {
        printf("ERROR: Tried to read %u bytes from chunk but read %zu\n", chunk->length, sz_read);
        return -1;
    }

    return 0;
}

director_t *director_load_file(char *fname) {
    fourcc_t tmp;
    size_t sz_read;
    director_chunk_t cur_chunk;
    director_t *result = malloc(sizeof(director_t));

    if (result == NULL) {
        printf("ERROR: Malloc failed.\n");
        return NULL;
    }

    result->fptr = fopen(fname, "r");
    if (result->fptr == NULL) {
        printf("ERROR: Failed to open \"%s\".\n", fname);
        return NULL;
    }

    /* Start by reading the FOURCC code */
    sz_read = fread(tmp.str, 4, 1, result->fptr);
    printf("FourCC: %.*s (%x)\n", 4, tmp.str, tmp.val);

    /* This detection isn't safe for BE systems */
    result->endianess = tmp.val == FOURCC_RIFX ? BIG_ENDIAN : LITTLE_ENDIAN;
    printf(result->endianess == BIG_ENDIAN ? "Big-endian file.\n" : "Little-endian file.\n");

    sz_read = fread(&result->length, 4, 1, result->fptr);
    result->length = result->endianess == BIG_ENDIAN ? SWAP_INT32(result->length) : result->length;

    /* Read next FOURCC code (codec) */
    sz_read = fread(result->codec.str, 4, 1, result->fptr);
    result->codec.val = result->endianess == LITTLE_ENDIAN ? SWAP_INT32(result->codec.val) : result->codec.val;
    printf("Type FourCC: %.*s (%x), length of file: %u\n", 4, result->codec.str, result->codec.val, result->length);

    if (result->codec.val != FOURCC_MV93 && result->codec.val != FOURCC_MC95) {
        printf("ERROR: Unsupported codec (maybe file was afterburned?).\n");
        return NULL;
    }

    /* Read and process imap chunk (comes next) */
    if (director_load_chunk(&cur_chunk, result, ftell(result->fptr)) != 0) {
        printf("ERROR: Failed to read imap chunk.\n");
        return NULL;
    }

    if (imap_process_chunk(&cur_chunk, &result->imap, result->endianess) != 0) {
        printf("ERROR: Failed to process imap chunk.\n");
        return NULL;
    }
    imap_print(&result->imap);
    /* Free the loaded data */
    free(cur_chunk.data);

    /* Read and process mmap chunk */
    if (director_load_chunk(&cur_chunk, result, result->imap.mmap_offset) != 0) {
        printf("ERROR: Failed to read mmap chunk.\n");
        return NULL;
    }

    if (mmap_process_chunk(&cur_chunk, &result->mmap, result->endianess) != 0) {
        printf("ERROR: Failed to process mmap chunk.\n");
        return NULL;
    }
    mmap_print(&result->mmap);
    mmap_print_entries(&result->mmap);
    /* Free the loaded data */
    free(cur_chunk.data);

    return result;


    /* Load Key table */
    //mmap_find_entry()

/*
    if (director_load_chunk(&cur_chunk, result, key_entry.offset) {
        printf("ERROR: Failed to read KEY chunk.\n");
        return NULL;
    }

    free(cur_chunk.data);
*/
    /* Dump PUBL */
/*
    if (mmap.publ_entry != NULL) {
        fseek(result->fptr, mmap.publ_entry->offset, SEEK_SET);
        if (loader_read_chunk(result->fptr, &cur_chunk, is_be) != 0) {
            printf("ERROR: Failed to read KEY chunk.\n");
            return NULL;
        }
        for (uint32_t i = 0; i < cur_chunk.len; i += 16) {
            printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
                    cur_chunk.data[i],  cur_chunk.data[i + 1],  cur_chunk.data[i + 2],  cur_chunk.data[i + 3],
                    cur_chunk.data[i + 4],  cur_chunk.data[i + 5],  cur_chunk.data[i + 6],  cur_chunk.data[i + 7],
                    cur_chunk.data[i + 8],  cur_chunk.data[i + 9],  cur_chunk.data[i + 10],  cur_chunk.data[i + 11],
                    cur_chunk.data[i + 12],  cur_chunk.data[i + 13],  cur_chunk.data[i + 14],  cur_chunk.data[i + 15]);
        }
    }
*/

    return result;
}
