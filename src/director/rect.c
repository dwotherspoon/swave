#include <director/rect.h>

#include <stdio.h>
#include <string.h>

int rect_process_data(uint8_t *data, rect_t *rect, endian_t endianness) {
    memcpy(rect, data, sizeof(rect_t));
    if (endianness == BIG_ENDIAN) {
        rect->top = SWAP_INT16(rect->top);
        rect->left = SWAP_INT16(rect->left);
        rect->bottom = SWAP_INT16(rect->bottom);
        rect->right = SWAP_INT16(rect->right);
    }
    return 0;
}

void rect_print(rect_t *rect) {
    printf("Rectangle: Top %i, Left %i, Bottom %i, Right %i (%i x %i)\n",
        rect->top, rect->left, rect->bottom, rect->right, rect->bottom - rect->top, rect->right - rect->left);
}
