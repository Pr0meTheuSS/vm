#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#define MEMORY_SIZE 4096 * 64

// Метаданные для переменной
typedef struct {
    char name[32];
    size_t offset;
    size_t size;
    int is_array;
    size_t length;
    int is_ready;
} Meta;

// Инициализация памяти
void init_memory();

// Освобождение памяти
void cleanup_memory();

// Получить указатель на память
void* get_shared_memory();

// Выделение смещения
size_t allocate_offset(size_t size);

#endif // MEMORY_MANAGER_H
