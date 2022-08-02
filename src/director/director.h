#ifndef _DIRECTOR_H
#define _DIRECTOR_H

#include <stdio.h>

#include <director/util.h>
#include <director/imap.h>
#include <director/mmap.h>
#include <director/keys.h>
#include <director/config.h>
#include <director/score.h>

#define INFLATE_BUF_SZ       512
#define FCDR_MAX_INFLATED_SZ 1024

typedef struct {
    FILE *fptr;
    /* Offset of RIFX in source file (for EXEs etc) */
    uint32_t offset;
    /* Information from RIFX */
    fourcc_t id;
    uint32_t length;
    fourcc_t codec;
    endian_t endianess;
    /* Chunks from file */
    imap_t imap;
    mmap_t mmap;
    keys_t keys;
    config_t config;
    score_t score;
/*

	castlist_t castlist;
	score_t score; */

} director_t;

director_t *director_load_file(char *fname);
void director_close_file(director_t *director);

#endif
