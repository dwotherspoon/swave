#include <director/keys.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void keys_process_entry(uint8_t *data, keys_entry_t *entry, endian_t endianness) {
    memcpy(entry, data, sizeof(keys_entry_t));
    if (endianness == BIG_ENDIAN) {
        entry->resource_id = SWAP_INT32(entry->resource_id);
        entry->owner_id = SWAP_INT32(entry->owner_id);
    } else {
        entry->type.val = SWAP_INT32(entry->type.val);
    } 
}

int keys_process_chunk(director_chunk_t *chunk, keys_t *keys, endian_t endianness) {
    size_t properties_length = (uint8_t*)&keys->entries - (uint8_t*)&keys->properties_length;
    keys_entry_t *cur_entry;
    uint8_t *data;

    if (chunk->type.val != FOURCC_KEYS) {
        printf("ERROR: Expected mmap chunk, got %.*s.\n", 4, chunk->type.str);
        return -1;
    }

    keys->length = chunk->length;

    memcpy(&keys->properties_length, chunk->data, properties_length);
    if (endianness == BIG_ENDIAN) {
        keys->properties_length = SWAP_INT16(keys->properties_length);
        keys->entry_length = SWAP_INT16(keys->entry_length);
        keys->max_entry_count = SWAP_INT32(keys->max_entry_count);
        keys->used_entry_count = SWAP_INT32(keys->used_entry_count);
    }

    if (keys->properties_length != properties_length) {
        printf("ERROR: Properties length mismatch, expected %zu got %u.\n", properties_length, keys->properties_length);
        return -1;
    }

    if (keys->entry_length != sizeof(keys_entry_t)) {
        printf("ERROR: Mmap entry length mismatch, expected %zu got %u.\n", sizeof(keys_entry_t), keys->entry_length);
        return -1;
    }

    keys->entries = (keys_entry_t*)malloc(sizeof(keys_entry_t) * keys->used_entry_count);
    if (keys->entries == NULL) {
        printf("ERROR: Failed to malloc keys entry table.\n");
        return -1;
    }

    cur_entry = keys->entries;
    data = chunk->data + properties_length;
    for (int32_t i = 0; i < keys->used_entry_count; i++) {
        keys_process_entry(data, cur_entry, endianness);
        data += keys->entry_length;
        cur_entry++;
    }

    return 0;
}

void keys_print(keys_t *keys) {
    printf("KEYS CHUNK INFORMATION:\n"
            "\tProperties Length: %u\n"
            "\tEntry Length:      %u\n"
            "\tMax Entry Count:   %i\n"
            "\tUsed Entry Count:  %i\n",
            keys->properties_length, keys->entry_length, keys->max_entry_count,
            keys->used_entry_count);
}

void keys_print_entry(keys_entry_t *entry) {
    printf("KEYS ENTRY INFORMATION:\n"
            "\tResource ID: %i\n"
            "\tOwner ID:    %i\n"
            "\tType:        %.*s\n",
            entry->resource_id, entry->owner_id, 4, entry->type.str);
}

void keys_print_entries(keys_t *keys) {
    if (keys->entries != NULL) {
        for (int32_t i = 0; i < keys->used_entry_count; i++) {
            printf("Entry %i:\n", i);
            keys_print_entry(&keys->entries[i]);
        }
    }
}
