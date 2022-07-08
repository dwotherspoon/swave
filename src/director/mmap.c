#include <director/mmap.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void mmap_process_entry(uint8_t *data, mmap_entry_t *entry, endian_t endianness) {
    memcpy(entry, data, sizeof(mmap_entry_t));
    if (endianness == BIG_ENDIAN) {
        entry->length = SWAP_INT32(entry->length);
        entry->offset = SWAP_INT32(entry->offset);
        entry->flags = SWAP_INT16(entry->flags);
        entry->reserved = SWAP_INT16(entry->reserved);
        entry->next_id = SWAP_INT32(entry->next_id);
    } else {
        entry->type.val = SWAP_INT32(entry->type.val);
    }
}

int mmap_process_chunk(director_chunk_t *chunk, mmap_t *mmap, endian_t endianness) {
    size_t properties_sz = (uint8_t*)&mmap->entries - (uint8_t*)&mmap->properties_length;
    mmap_entry_t *cur_entry;
    uint8_t *data;

    //mmap->key_entry = NULL;

    if (chunk->id.val != FOURCC_MMAP) {
        printf("ERROR: Expected mmap chunk, got %.*s.\n", 4, chunk->id.str);
        return -1;
    }

    mmap->id = chunk->id;
    mmap->length = chunk->length;

    memcpy(&mmap->properties_length, chunk->data, properties_sz);
    if (endianness == BIG_ENDIAN) {
        mmap->properties_length = SWAP_INT16(mmap->properties_length);
        mmap->entry_length = SWAP_INT16(mmap->entry_length);
        mmap->max_resource_count = SWAP_INT32(mmap->max_resource_count);
        mmap->used_resource_count = SWAP_INT32(mmap->used_resource_count);
        mmap->first_junk_id = SWAP_INT32(mmap->first_junk_id);
        mmap->old_mmap_id = SWAP_INT32(mmap->old_mmap_id);
        mmap->first_free_id = SWAP_INT32(mmap->first_free_id);
    }

    if (mmap->properties_length != properties_sz) {
        printf("ERROR: Properties length mismatch, expected %zu got %u.\n", properties_sz, mmap->properties_length);
        return - 1;
    }

    if (mmap->entry_length != sizeof(mmap_entry_t)) {
        printf("ERROR: Mmap entry length mismatch, expected %zu got %u.\n", sizeof(mmap_entry_t), mmap->entry_length);
    }

    mmap->entries = (mmap_entry_t*)malloc(sizeof(mmap_entry_t) * mmap->used_resource_count);
    if (mmap->entries == NULL) {
        printf("ERROR: Failed to malloc mmap entry table.\n");
        return -1;
    }

    cur_entry = mmap->entries;
    data = chunk->data + properties_sz;
    for (int32_t i = 0; i < mmap->used_resource_count; i++) {
        mmap_process_entry(data, cur_entry, endianness);
        data += mmap->entry_length;
        cur_entry++;
    }

    return 0;
}

void mmap_print(mmap_t *mmap) {
    printf("MMAP CHUNK INFORMATION:\n"
            "\tProperties Length:   %u\n"
            "\tEntry Length:        %u\n"
            "\tMax Resource Count:  %i\n"
            "\tUsed Resource Count: %i\n"
            "\tFirst Junk ID:       %i\n"
            "\tOld MMAP ID:         %i\n"
            "\tFirst Free ID:       %i\n",
            mmap->properties_length, mmap->entry_length, mmap->max_resource_count,
            mmap->used_resource_count, mmap->first_junk_id, mmap->old_mmap_id,
            mmap->first_free_id);
}

void mmap_print_entry(mmap_entry_t *entry) {
    printf("MMAP ENTRY INFORMATION:\n"
            "\tType:     %.*s\n"
            "\tLength    %u\n"
            "\tOffset:   %u\n"
            "\tFlags:    %x\n"
            "\tReserved: %x\n"
            "\tNext ID:  %i\n",
            4, entry->type.str, entry->length, entry->offset,
            entry->flags, entry->reserved, entry->next_id);
}

void mmap_print_entries(mmap_t *mmap) {
    if (mmap->entries != NULL) {
        for (int32_t i = 0; i < mmap->used_resource_count; i++) {
            printf("Entry %i:\n", i);
            mmap_print_entry(&mmap->entries[i]);
        }
    }
}
