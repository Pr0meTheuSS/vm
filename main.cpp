#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define MEMORY_SIZE 4096

// Метаданные для переменной
typedef struct {
    char name[32];    // Имя переменной
    size_t offset;    // Смещение значения от начала блока
    size_t size;      // Размер значения
    int is_ready;     // Готовность переменной (1 - да, 0 - нет)
} Meta;

// Указатель на начало памяти
static void* shared_memory = NULL;

// Макросы для работы с переменными
#define DEFINE_VAR(v_name, type) \
    Meta v_name##_meta = {.name = #v_name, .offset = ALLOCATE_OFFSET(sizeof(type)), .size = sizeof(type), .is_ready = 0}

#define SET_VAR(name, value) \
    do { \
        *((uint8_t*)shared_memory + name##_meta.offset) = value; \
        name##_meta.is_ready = 1; \
    } while (0)

#define GET_VAR(name, ptr) \
    do { \
        if (!name##_meta.is_ready) { \
            exit(1); \
        } \
        *ptr = *((uint8_t*)shared_memory + name##_meta.offset); \
    } while (0)

// Текущий смещение для размещения переменных
size_t current_offset = 0;

// Макрос для вычисления смещения переменной
#define ALLOCATE_OFFSET(size) ({ \
    size_t offset = current_offset; \
    current_offset += (size); \
    if (current_offset > MEMORY_SIZE) { \
        fprintf(stderr, "Memory overflow: not enough space!\n"); \
        exit(1); \
    } \
    offset; \
})

// Инициализация памяти
void init_memory() {
    shared_memory = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
}

// Освобождение памяти
void cleanup_memory() {
    if (munmap(shared_memory, MEMORY_SIZE) == -1) {
        perror("munmap failed");
    }
}

int main() {
    init_memory();

    // Определение переменных
    DEFINE_VAR(side_a, double);
    DEFINE_VAR(side_b, double);
    DEFINE_VAR(side_c, double);
    DEFINE_VAR(perimeter, double);
    DEFINE_VAR(area, double);

    // Установка значений
    SET_VAR(side_a, 3.0);
    SET_VAR(side_b, 4.0);
    SET_VAR(side_c, 5.0);

    if (side_a_meta.is_ready && side_b_meta.is_ready && side_c_meta.is_ready) {
        double a, b, c;
        GET_VAR(side_a, &a);
        GET_VAR(side_b, &b);
        GET_VAR(side_c, &c);
        printf("%f, %f, %f", a, b, c);

        double p = a + b + c;
        SET_VAR(perimeter, p);

        double s = p / 2.0;
        double area_value = std::sqrt(s * (s - a) * (s - b) * (s - c));
        SET_VAR(area, area_value);

        // Вывод
        printf("\nПериметр: %.2f", p);
        printf("\nПлощадь: %.2f", area_value);
    } else {
        fprintf(stderr, "Не все переменные готовы для вычислений.\n");
    }

    // Очистка
    cleanup_memory();
    return 0;
}
