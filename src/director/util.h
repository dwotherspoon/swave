#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

/* Define some common types */
#define FOURCC_RIFX 0x58464952
#define FOURCC_XFIR 0x52494658
#define FOURCC_MV93 0x3339564d
#define FOURCC_MC95 0x3539434d
#define FOURCC_FGDM 0x4d444746
#define FOURCC_FGDC 0x4d444746
#define FOURCC_IMAP 0x70616d69
#define FOURCC_MMAP 0x70616d6d
#define FOURCC_free 0x65657266
#define FOURCC_KEY  0x2a59454b
#define FOURCC_PUBL 0x4c425550

#define SWAP_INT16(X) ((((X) & 0xff) << 8)  |\
                       (((X) & 0xff00) >> 8))

#define SWAP_INT32(X) ((((X) & 0xff) << 24)      |\
                       (((X) & 0xff00) << 8)     |\
                       (((X) & 0xff0000) >> 8)   |\
                       (((X) & 0xff000000) >> 24))

typedef union {
    uint32_t val;
    char str[4];
} fourcc_t;

typedef enum {
    LITTLE_ENDIAN,
    BIG_ENDIAN
} endian_t;

typedef struct {
    fourcc_t id;
    uint32_t length;
    uint8_t *data;
} director_chunk_t;

#endif
