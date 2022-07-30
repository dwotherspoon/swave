#include <director/director.h>

#include <stdlib.h>
#include <zlib.h>

int director_load_chunk(director_chunk_t *chunk, director_t *director, uint32_t offset) {
    size_t sz_read;
    fseek(director->fptr, offset, SEEK_SET);

    sz_read = fread(chunk->type.str, 4, 1, director->fptr);
    if (sz_read != 1) {
        printf("ERROR: Failed to read id of chunk.\n");
        return -1;
    }
    chunk->type.val = director->endianess == LITTLE_ENDIAN ? SWAP_INT32(chunk->type.val) : chunk->type.val;

    sz_read = fread(&chunk->length, 4, 1, director->fptr);
    if (sz_read != 1) {
        printf("ERROR: Failed to read length of chunk.\n");
        return -1;
    }
    chunk->length = director->endianess == BIG_ENDIAN ? SWAP_INT32(chunk->length) : chunk->length;

    printf("Read chunk with FourCC: %.*s (%x), length: %u\n", 4, chunk->type.str, chunk->type.val, chunk->length);

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

int director_load_compressed_chunk(director_chunk_t *chunk, director_t *director, uint32_t offset) {
    fourcc_t tmp;
    size_t sz_read, sz_used;
    uint32_t deflated_len, inflated_len, compression_type;
    uint8_t buf[INFLATE_BUF_SZ];
    int32_t inflate_result;
    z_stream inflate_stream;

    fseek(director->fptr, offset, SEEK_SET);

    sz_read = fread(tmp.str, 4, 1, director->fptr);
    if (sz_read != 1) {
        return -1;
    }
    tmp.val = director->endianess == LITTLE_ENDIAN ? SWAP_INT32(tmp.val) : tmp.val;
    printf("FourCC: %.*s (%x)\n", 4, tmp.str, tmp.val);

    /* First read var int uncompressed length */
    sz_read = fread(buf, 1, INFLATE_BUF_SZ, director->fptr);
    sz_used = util_read_varint(buf, &deflated_len); 
    sz_used += util_read_varint(buf + sz_used, &compression_type); 
    sz_used += util_read_varint(buf + sz_used, &inflated_len); 

    printf("INFO: Deflated length: %i, compression type: %x, inflated length: %i\n", deflated_len, compression_type, inflated_len);

    chunk->data = malloc(deflated_len + 100);
    if (chunk->data == NULL) {
        printf("This malloc failed\n");
        return -1;
    }

    inflate_stream.zalloc = Z_NULL;
    inflate_stream.zfree = Z_NULL;
    inflate_stream.opaque = Z_NULL;

    inflate_stream.avail_out = inflated_len + 100;
    inflate_stream.next_out = chunk->data;

    inflate_stream.avail_in = sz_read - sz_used;
    inflate_stream.next_in = buf + sz_used;
     
    inflate_result = inflateInit(&inflate_stream);
    if (inflate_result != Z_OK) {
        printf("Error here\n");
        return -1;
    }

    do {
        if (inflate_stream.avail_in == 0) {
            sz_read = fread(buf, 1, INFLATE_BUF_SZ, director->fptr);
            inflate_stream.avail_in = sz_read;
            inflate_stream.next_in = buf;
        }
        inflate_result = inflate(&inflate_stream, Z_NO_FLUSH);
        printf("Inflate result = %i, avail_in %u, total_in %lu, total_out %lu\n", inflate_result, inflate_stream.avail_in, inflate_stream.total_in, inflate_stream.total_out);
        printf("%s\n", inflate_stream.msg);
        if (inflate_result < 0) {
            return -1;
        }
    } while (inflate_result != Z_STREAM_END);

    inflateEnd(&inflate_stream);


    // printf("INFO: ZLib inflated %i bytes to %i (total = %i) bytes\n", inflate_stream.total_in, inflate_stream.total_out, inflate_full_length);

    /* Rewind FP by  xyz */

    return 0;

}

int diretor_read_mmap(director_t *director) {
    director_chunk_t cur_chunk;

    /* Read and process imap chunk (comes next) */
    if (director_load_chunk(&cur_chunk, director, ftell(director->fptr)) != 0) {
        printf("ERROR: Failed to read imap chunk.\n");
        return -1;
    }

    if (imap_process_chunk(&cur_chunk, &director->imap, director->endianess) != 0) {
        printf("ERROR: Failed to process imap chunk.\n");
        return -1;
    }
    imap_print(&director->imap);

    /* Free the loaded data */
    free(cur_chunk.data);

    /* Read and process mmap chunk */
    if (director_load_chunk(&cur_chunk, director, director->imap.mmap_offset) != 0) {
        printf("ERROR: Failed to read mmap chunk.\n");
        return -1;
    }

    if (mmap_process_chunk(&cur_chunk, &director->mmap, director->endianess) != 0) {
        printf("ERROR: Failed to process mmap chunk.\n");
        return -1;
    }
    mmap_print(&director->mmap);
    //mmap_print_entries(&director->mmap);

    /* Free the loaded data */
    free(cur_chunk.data);

    return 0;
}

int director_read_abmp(director_t *director) {
    fourcc_t tmp;
    size_t sz_read, sz_used, sz_len;
    uint8_t buf[INFLATE_BUF_SZ] = {0};
    uint8_t *ptr;
    int32_t inflate_result;
    uint32_t fcdr_deflated_len, fver_len, file_version;
    director_chunk_t cur_chunk;
    z_stream inflate_stream;

    printf("INFO: Zlib version %s\n", zlib_version);

    sz_read = fread(tmp.str, 4, 1, director->fptr);
    tmp.val = director->endianess == LITTLE_ENDIAN ? SWAP_INT32(tmp.val) : tmp.val;
    if (tmp.val != FOURCC_Fver) {
        printf("ERROR: Expected Fver FourCC.\n");
        return -1;
    }
    printf("FourCC: %.*s (%x)\n", 4, tmp.str, tmp.val);

    /* Read 256 bytes, this should always be enough for all of FVER (usually < 20 bytes) */

    sz_read = fread(buf, 1, INFLATE_BUF_SZ, director->fptr);
    ptr = buf;
    sz_len = util_read_varint(ptr, &fver_len);
    sz_used = sz_len;

    if ((sz_read - sz_used) < fver_len) {
        printf("ERROR: Fver longer than buffer (buffer length: %zu, Fver length: %u).\n", sz_read, fver_len);
    }

    ptr = buf + sz_used;
    sz_used += util_read_varint(ptr, &file_version);

    printf("INFO: Fver file version = 0x%x.\n", file_version);

    if (file_version >= 0x401) {
        uint32_t imap_version, director_version;

        ptr = buf + sz_used;
        sz_used += util_read_varint(ptr, &imap_version);

        ptr = buf + sz_used;
        sz_used += util_read_varint(ptr, &director_version);
		printf("INFO: Fver imap version: %u, director version: 0x%x.\n", imap_version, director_version);
	}

	if (file_version >= 0x501) {
        ptr = buf + sz_used;
        uint8_t string_len = *ptr++;
        sz_used += 1 + string_len;
        printf("String_len = %i\n", string_len);
        printf("INFO: Fver version string: %.*s.\n", string_len, ptr);
	}

    if ((sz_used - sz_len) != fver_len) {
        printf("ERROR\n");
    }

    /* Rewind FP to exact end of Fver chunk */
    fseek(director->fptr, -sz_read + sz_used, SEEK_CUR);

    /* Fcdr chunk is next, this is a special case and has no uncompressed length */

    sz_read = fread(tmp.str, 4, 1, director->fptr);
    tmp.val = director->endianess == LITTLE_ENDIAN ? SWAP_INT32(tmp.val) : tmp.val;
    if (tmp.val != FOURCC_Fcdr) {
        printf("ERROR: Expected Fcdr FourCC.\n");
        return -1;
    }
    printf("FourCC: %.*s (%x)\n", 4, tmp.str, tmp.val);

    /* First read var int uncompressed length */
    sz_read = fread(buf, 1, INFLATE_BUF_SZ, director->fptr);
    sz_used = util_read_varint(buf, &fcdr_deflated_len); 

    /* 1024 bytes should always be enough */
    ptr = malloc(FCDR_MAX_DEFLATED_SZ);
    if (ptr == NULL) {
        printf("ERROR: Malloc failed.\n");
        return -1;
    }

    inflate_stream.zalloc = Z_NULL;
    inflate_stream.zfree = Z_NULL;
    inflate_stream.opaque = Z_NULL;

    inflate_stream.avail_out = FCDR_MAX_DEFLATED_SZ;
    inflate_stream.next_out = ptr;

    inflate_stream.avail_in = sz_read - sz_used;
    inflate_stream.next_in = buf + sz_used;
     
    inflate_result = inflateInit(&inflate_stream);
    if (inflate_result != Z_OK) {
        printf("Error here\n");
        return -1;
    }

    do {
        if (inflate_stream.avail_in == 0) {
            sz_read = fread(buf, 1, INFLATE_BUF_SZ, director->fptr);
            inflate_stream.avail_in = sz_read;
            inflate_stream.next_in = buf;
        }
        inflate_result = inflate(&inflate_stream, Z_NO_FLUSH);
        if (inflate_result < 0) {
            return -1;
        }
    } while (inflate_result != Z_STREAM_END);

    inflateEnd(&inflate_stream);

    printf("INFO: Inflated Fcdr chunk from %lu to %lu bytes.\n", inflate_stream.total_in, inflate_stream.total_out);

    /* Rewind any unused bytes */
    fseek(director->fptr, -inflate_stream.avail_in, SEEK_CUR);

    /* Read ABMP chunk */
    director_load_compressed_chunk(&cur_chunk, director, ftell(director->fptr));


    // return director_load_compressed_chunk(&cur_chunk, director, ftell(director->fptr));
    return -1;

}

director_t *director_load_file(char *fname) {
    fourcc_t tmp;
    size_t sz_read;
    director_chunk_t cur_chunk;
    director_t *result = malloc(sizeof(director_t));
    int32_t mmap_result_id;

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

    if (result->codec.val == FOURCC_MV93 && result->codec.val != FOURCC_MC95) {
        if (diretor_read_mmap(result) != 0) {
            printf("ERROR: Failed reading uncompressed file.\n");
            return NULL;
        }
    }
    else if (result->codec.val == FOURCC_FGDM || result->codec.val == FOURCC_FGDC) {
        if (director_read_abmp(result) != 0) {
            printf("ERROR: Failed reading compressed file.\n");
            return NULL;
        }
    } else {
        printf("ERROR: Unsupported codec %.*s.\n", 4, result->codec.str);
        return NULL;
    }

    /* Load key table */
    tmp.val = FOURCC_KEYS;
    mmap_result_id = mmap_find_entry(&result->mmap, tmp, 0);
    if (mmap_result_id == -1) {
        printf("ERROR: Failed to find keys table in mmap.\n");
        return NULL;
    }

    mmap_print_entry(&result->mmap.entries[mmap_result_id]);

    if (director_load_chunk(&cur_chunk, result, result->mmap.entries[mmap_result_id].offset) != 0) {
        printf("ERROR: Failed to read keys chunk.\n");
        return NULL;
    }

    if (keys_process_chunk(&cur_chunk, &result->keys, result->endianess) != 0) {
        printf("ERROR: Failed to process keys chunk.\n");
        return NULL;
    }

    keys_print(&result->keys);
    //keys_print_entries(&result->keys);

    /* Free the loaded data */
    free(cur_chunk.data);

    /* Load video info */
    tmp.val = FOURCC_VWCF;
    mmap_result_id = mmap_find_entry(&result->mmap, tmp, 0);
    if (mmap_result_id == -1) {
        printf("INFO: VWCF not found, trying DRCF\n");
        tmp.val = FOURCC_DRCF;
    }
    mmap_result_id = mmap_find_entry(&result->mmap, tmp, 0);
    if (mmap_result_id == -1) {
        printf("ERROR: Failed to find config entry (VWCF/DRCF).\n");
        return NULL;
    }

    if (director_load_chunk(&cur_chunk, result, result->mmap.entries[mmap_result_id].offset) != 0) {
        printf("ERROR: Failed to load config chunk.\n");
        return NULL;
    }

    if (config_process_chunk(&cur_chunk, &result->config, result->endianess) != 0)  {
        printf("ERROR: Faled to process config chunk.\n");
        return NULL;
    }

    config_print(&result->config);

    /* Free the loaded data */
    free(cur_chunk.data);

    /* Load video score */
    tmp.val = FOURCC_VWSC;
    mmap_result_id = mmap_find_entry(&result->mmap, tmp, 0);
    if (mmap_result_id == -1) {
        printf("ERROR: Failed to find score entry (VWSC).\n");
        return NULL;
    }

    if (director_load_chunk(&cur_chunk, result, result->mmap.entries[mmap_result_id].offset) != 0) {
        printf("ERROR: Failed to load score chunk.\n");
        return NULL;
    }

    if (score_process_chunk(&cur_chunk, &result->config, result->endianess) != 0) {
        printf("ERROR: Failed to process score chunk.\n");
        return NULL;
    }

    /* Free the loaded data */
    free(cur_chunk.data);

    return result;
}
