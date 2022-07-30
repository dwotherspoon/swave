#ifndef _CONFIG_H
#define _CONFIG_H

#include <director/util.h>
#include <director/rect.h>

typedef struct __attribute__((__packed__)) {
    uint32_t length;
    uint16_t properties_length;
    uint16_t file_version;
    rect_t movie_rect;
    int16_t min_cast_member;
    int16_t max_cast_memember;
    int8_t framerate;
} config_t;

int config_process_chunk(director_chunk_t *chunk, config_t *config, endian_t endianness);
void config_print(config_t *config);

#endif
