#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define FOURCC_RIFX 0x58464952
#define FOURCC_XFIR 0x52494658
#define FOURCC_MV93 0x3339564d
#define FOURCC_MC95 0x3539434d
#define FOURCC_FGDM 0x4d444746
#define FOURCC_FGDC 0x4d444746
#define FOURCC_IMAP 0x70616d69
#define FOURCC_MMAP 0x70616d6d

#define SWAP_INT16(X) ((((X) & 0xff) << 8) |\
                       (((X) & 0xff00) >> 8))

#define SWAP_INT32(X) ((((X) & 0xff) << 24)    |\
                       (((X) & 0xff00) << 8)   |\
                       (((X) & 0xff0000) >> 8) |\
                       (((X) & 0xff000000) >> 24))

union fourcc_t {
    char str[4];
    uint32_t val;
};

struct loader_chunk_t {
    union fourcc_t id;
    uint32_t len;
    uint8_t *data;
};

struct loader_imap_t {
    uint32_t mmap_count;
    uint32_t mmap_offset;
    uint32_t mmap_version;
    uint32_t reserved;
    uint32_t reserved1;
    uint32_t reserved2;
};

struct loader_mmap_t {
    uint16_t properties_sz;
    uint16_t resource_sz;
    uint32_t max_resource_count;
    uint32_t used_resource_count;
    uint32_t first_junk_id;
    uint32_t old_mmap_id;
    uint32_t first_free_id;
    struct loader_resource_t *entries;
};

int loader_load_file(char *fname);
