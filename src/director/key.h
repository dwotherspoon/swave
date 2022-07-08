#ifndef _KEY_H
#define _KEY_H

#include <director/util.h>

typedef struct {
    fourcc_t id;
    uint32_t length;
    uint16_t properties_sz;
    uint16_t key_sz;
    uint32_t max_key_count;
    uint32_t used_key_count;
} key_t;

#endif