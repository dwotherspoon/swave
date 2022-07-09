#include <director/config.h>

#include <stdio.h>
#include <string.h>

int config_process_chunk(director_chunk_t *chunk, config_t *config, endian_t endianness) {
    uint8_t *ptr;
    if (chunk->type.val != FOURCC_VWCF && chunk->type.val != FOURCC_DRCF) {
        printf("ERROR: Expected VWCF/DRCF chunk, got %.*s\n", 4, chunk->type.str);
        return -1;
    }

    config->length = chunk->length;
    ptr = chunk->data;

    memcpy(&config->properties_length, ptr, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    memcpy(&config->file_version, ptr, sizeof(uint16_t));
    ptr += sizeof(uint16_t);

    rect_process_data(ptr, &config->movie_rect, BIG_ENDIAN);
    rect_print(&config->movie_rect);

    /* Properties always big endian */
    config->properties_length = SWAP_INT16(config->properties_length);
    config->file_version = SWAP_INT16(config->file_version);

    return 0;
}

void config_print(config_t *config) {
    printf("CONFIG CHUNK INFORMATION:\n"
            "\tProperties Length:   %u (%x)\n"
            "\tFile version:  %x (v%u)\n",
            config->properties_length, config->properties_length, config->file_version, util_decode_version(config->file_version));
}
