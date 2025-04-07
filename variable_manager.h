#ifndef VARIABLE_MANAGER_H
#define VARIABLE_MANAGER_H

#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>

#define MAX_CALLBACKS 10

typedef void (*ready_callback_t)(void);

typedef struct {
    const char* name;
    size_t offset;
    size_t size;
    int is_array;
    size_t length;
    volatile int is_ready;
    ready_callback_t callbacks[MAX_CALLBACKS];
    int callback_count;
} Meta;

void register_ready_callback(Meta* meta, ready_callback_t callback) {
    if (meta->callback_count < MAX_CALLBACKS) {
        meta->callbacks[meta->callback_count++] = callback;
    }
}

void mark_ready(Meta* meta) {
    meta->is_ready = 1;
    for (int i = 0; i < meta->callback_count; i++) {
        meta->callbacks[i]();
    }
}

#define DEFINE_VAR(v_name, type) \
    Meta v_name##_meta = {.name = #v_name, .offset = allocate_offset(sizeof(type)), .size = sizeof(type), .is_array = 0, .is_ready = 0, .callback_count = 0}

#define DEFINE_ARRAY(v_name, type, len) \
    Meta v_name##_meta = {.name = #v_name, .offset = allocate_offset(sizeof(type) * (len)), .size = sizeof(type), .is_array = 1, .length = (len), .is_ready = 0, .callback_count = 0}

#define SET_VAR(name, value) \
    do { \
        *((typeof(value)*)((uint8_t*)get_shared_memory() + name##_meta.offset)) = value; \
        mark_ready(&name##_meta); \
    } while (0)

#define GET_VAR(name, ptr) \
    do { \
        while (!name##_meta.is_ready); \
        *ptr = *((typeof(*ptr)*)((uint8_t*)get_shared_memory() + name##_meta.offset)); \
    } while (0)

#define SET_ARRAY(name, index, value) \
    do { \
        if (index >= name##_meta.length) { \
            fprintf(stderr, "Index out of bounds for array '%s'.\n", #name); \
            exit(1); \
        } \
        ((typeof(value)*)((uint8_t*)get_shared_memory() + name##_meta.offset))[index] = value; \
        mark_ready(&name##_meta); \
    } while (0)

#define GET_ARRAY(name, index, ptr) \
    do { \
        while (!name##_meta.is_ready); \
        *ptr = ((typeof(*ptr)*)((uint8_t*)get_shared_memory() + name##_meta.offset))[index]; \
    } while (0)

#define DEFINE_MATRIX(mat_name, type, rows, cols) \
    Meta mat_name##_meta = {                      \
        .name = #mat_name,                        \
        .offset = allocate_offset(sizeof(type) * (rows) * (cols)), \
        .size = sizeof(type),                     \
        .is_array = 1,                            \
        .length = (rows) * (cols),                \
        .is_ready = 0,                            \
        .callback_count = 0                       \
    }

#define SET_MATRIX_ELEMENT(mat_name, row, col, cols, value)                                  \
    ((typeof(value)*)((uint8_t*)get_shared_memory() + (mat_name##_meta).offset))[(row) * (cols) + (col)] = (value)

#define GET_MATRIX_ELEMENT(mat_name, row, col, cols, ptr)                                   \
    *(ptr) = ((typeof(*(ptr))*)((uint8_t*)get_shared_memory() + (mat_name##_meta).offset))[(row) * (cols) + (col)]

#define INIT_MATRIX(mat_name, type, rows, cols, value)                   \
    do {                                                                 \
        for (int i = 0; i < (rows); i++) {                               \
            for (int j = 0; j < (cols); j++) {                           \
                SET_MATRIX_ELEMENT(mat_name, i, j, cols, (type)(value)); \
            }                                                            \
        }                                                                \
        mark_ready(&mat_name##_meta);                                    \
    } while (0)

#define GET_MATRIX_SLICE(mat_name, type, start_row, end_row, start_col, end_col, cols, out_ptr) \
    do {                                                                                         \
        int idx = 0;                                                                             \
        for (int i = (start_row); i < (end_row); i++) {                                          \
            for (int j = (start_col); j < (end_col); j++) {                                      \
                (out_ptr)[idx++] = ((type*)((uint8_t*)get_shared_memory() + mat_name##_meta.offset))[(i) * (cols) + (j)]; \
            }                                                                                    \
        }                                                                                        \
    } while (0)

void* get_data_from_stream(FILE* stream, size_t size) {
    void* buffer = malloc(size);
    fread(buffer, 1, size, stream);
    return buffer;
}

#endif // VARIABLE_MANAGER_H
