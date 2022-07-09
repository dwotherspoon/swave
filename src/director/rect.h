#ifndef _RECT_H
#define _RECT_H

#include <director/util.h>

typedef struct __attribute__((__packed__)) {
	int16_t top;
	int16_t left;
	int16_t bottom;
	int16_t right;
} rect_t;

int rect_process_data(uint8_t *data, rect_t *rect, endian_t endianness);
void rect_print(rect_t *rect);

#endif
