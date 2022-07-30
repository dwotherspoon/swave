#ifndef _SCORE_H
#define _SCORE_H

#include <director/util.h>

typedef struct {
    uint32_t length;
} score_t;

int score_process_chunk(director_chunk_t *chunk, score_t *score, endian_t endianness);
void score_print(score_t *score);

#endif
