#ifndef VARIABLE_MANAGER_H
#define VARIABLE_MANAGER_H

#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>

// Макросы для работы с переменными
#define DEFINE_VAR(v_name, type) \
    Meta v_name##_meta = {.name = #v_name, .offset = allocate_offset(sizeof(type)), .size = sizeof(type), .is_array = 0, .is_ready = 0}

#define DEFINE_ARRAY(v_name, type, len) \
    Meta v_name##_meta = {.name = #v_name, .offset = allocate_offset(sizeof(type) * (len)), .size = sizeof(type), .is_array = 1, .length = (len), .is_ready = 0}

#define SET_VAR(name, value) \
    do { \
        *((typeof(value)*)((uint8_t*)get_shared_memory() + name##_meta.offset)) = value; \
        name##_meta.is_ready = 1; \
    } while (0)

#define GET_VAR(name, ptr) \
    do { \
        if (!name##_meta.is_ready) { \
            fprintf(stderr, "Variable '%s' is not ready.\n", #name); \
            exit(1); \
        } \
        *ptr = *((typeof(*ptr)*)((uint8_t*)get_shared_memory() + name##_meta.offset)); \
    } while (0)

#define SET_ARRAY(name, index, value) \
    do { \
        if (index >= name##_meta.length) { \
            fprintf(stderr, "Index out of bounds for array '%s'.\n", #name); \
            exit(1); \
        } \
        ((typeof(value)*)((uint8_t*)get_shared_memory() + name##_meta.offset))[index] = value; \
        name##_meta.is_ready = 1; \
    } while (0)

#define GET_ARRAY(name, index, ptr) \
    do { \
        if (!name##_meta.is_ready) { \
            fprintf(stderr, "Array '%s' is not ready.\n", #name); \
            exit(1); \
        } \
        *ptr = ((typeof(*ptr)*)((uint8_t*)get_shared_memory() + name##_meta.offset))[index]; \
    } while (0)

#endif // VARIABLE_MANAGER_H
