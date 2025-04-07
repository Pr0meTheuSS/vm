#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/resource.h>

// ============ НАСТРОЙКИ ============
#define MATRIX_SIZE 512
int BLOCK_SIZE = 64; // Можно передать через аргумент

// ============ ПАМЯТЬ ==============
#define MAX_VARIABLES 128
#define MEMORY_POOL_SIZE (1024 * 1024) // 1 MB

typedef struct {
    char name[64];
    size_t offset;
    size_t size;
    int is_array;
    int is_ready;
} Meta;

static Meta variables[MAX_VARIABLES];
static int variable_count = 0;
static size_t current_offset = 0;
static uint8_t* memory_pool = NULL;

void init_memory() {
    if (!memory_pool) {
        memory_pool = (uint8_t*)malloc(MEMORY_POOL_SIZE);
        if (!memory_pool) {
            perror("Failed to allocate memory pool");
            exit(1);
        }
        memset(memory_pool, 0, MEMORY_POOL_SIZE);
    }
}

void* get_or_create_block(const char* name, size_t size) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return memory_pool + variables[i].offset;
        }
    }

    if (variable_count >= MAX_VARIABLES || current_offset + size > MEMORY_POOL_SIZE) {
        fprintf(stderr, "Memory limit or variable limit exceeded.\n");
        exit(1);
    }

    strncpy(variables[variable_count].name, name, 63);
    variables[variable_count].offset = current_offset;
    variables[variable_count].size = size;
    variables[variable_count].is_array = 1;
    variables[variable_count].is_ready = 1;
    current_offset += size;
    return memory_pool + variables[variable_count++].offset;
}

// ============ ПРОФИЛИРОВАНИЕ ==============

void print_mem_usage() {
    FILE* f = fopen("/proc/self/status", "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0 || strncmp(line, "VmSize:", 7) == 0) {
            printf("%s", line);
        }
    }
    fclose(f);
}

void print_peak_rss() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Peak RSS: %ld KB\n", usage.ru_maxrss);
}

// ============ МАТРИЦЫ ==============

void load_block_from_file(FILE* f, double* block, int row_start, int col_start, int stride, int block_size) {
    fseek(f, 0, SEEK_SET);
    for (int i = 0; i < block_size; i++) {
        fseek(f, sizeof(double) * ((row_start + i) * stride + col_start), SEEK_SET);
        fread(&block[i * block_size], sizeof(double), block_size, f);
    }
}

void blocked_matrix_multiply(FILE* fileA, FILE* fileB) {
    int N = MATRIX_SIZE;
    int B = BLOCK_SIZE;

    init_memory();

    double* blockA = (double*)get_or_create_block("blockA", sizeof(double) * B * B);
    double* blockB = (double*)get_or_create_block("blockB", sizeof(double) * B * B);
    double* blockC = (double*)get_or_create_block("blockC", sizeof(double) * B * B);

    double* result = malloc(sizeof(double) * N * N);
    if (!result) {
        fprintf(stderr, "Failed to allocate result matrix\n");
        exit(1);
    }
    memset(result, 0, sizeof(double) * N * N);

    for (int ii = 0; ii < N; ii += B) {
        for (int jj = 0; jj < N; jj += B) {
            memset(blockC, 0, sizeof(double) * B * B);

            for (int kk = 0; kk < N; kk += B) {
                load_block_from_file(fileA, blockA, ii, kk, N, B);
                load_block_from_file(fileB, blockB, kk, jj, N, B);

                for (int i = 0; i < B; i++) {
                    for (int j = 0; j < B; j++) {
                        for (int k = 0; k < B; k++) {
                            blockC[i * B + j] += blockA[i * B + k] * blockB[k * B + j];
                        }
                    }
                }
            }

            for (int i = 0; i < B; i++) {
                for (int j = 0; j < B; j++) {
                    result[(ii + i) * N + (jj + j)] = blockC[i * B + j];
                }
            }
        }
    }

    free(result);
}

// ========== MAIN ============
int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s matrixA.bin matrixB.bin [block_size]\n", argv[0]);
        return 1;
    }

    if (argc == 4) {
        BLOCK_SIZE = atoi(argv[3]);
        if (BLOCK_SIZE <= 0 || MATRIX_SIZE % BLOCK_SIZE != 0) {
            fprintf(stderr, "Invalid block size. It must divide %d.\n", MATRIX_SIZE);
            return 1;
        }
    }

    FILE* fA = fopen(argv[1], "rb");
    FILE* fB = fopen(argv[2], "rb");

    if (!fA || !fB) {
        perror("File open error");
        return 1;
    }

    printf("Block size: %d\n", BLOCK_SIZE);
    printf("Memory usage before:\n");
    print_mem_usage();

    clock_t start = clock();
    blocked_matrix_multiply(fA, fB);
    clock_t end = clock();

    printf("Memory usage after:\n");
    print_mem_usage();
    print_peak_rss();

    printf("Time: %.3f sec\n", (double)(end - start) / CLOCKS_PER_SEC);

    fclose(fA);
    fclose(fB);
    return 0;
}
