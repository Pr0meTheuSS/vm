#include <time.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <stdio.h>

#define MATRIX_SIZE 512
int BLOCK_SIZE = 64; // глобальная переменная

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

void blocked_matrix_multiply() {
    int N = MATRIX_SIZE;
    int B = BLOCK_SIZE;

    define_variable("blockA", sizeof(double) * B * B, 1, 0);
    define_variable("blockB", sizeof(double) * B * B, 1, 0);
    define_variable("blockC", sizeof(double) * B * B, 1, 0);

    double* blockA = (double*)(memory_pool + variables[0].offset);
    double* blockB = (double*)(memory_pool + variables[1].offset);
    double* blockC = (double*)(memory_pool + variables[2].offset);

    // Инициализация результата
    double* result = malloc(sizeof(double) * N * N);
    memset(result, 0, sizeof(double) * N * N);

    for (int ii = 0; ii < N; ii += B) {
        for (int jj = 0; jj < N; jj += B) {
            memset(blockC, 0, sizeof(double) * B * B); // очищаем блок результата

            for (int kk = 0; kk < N; kk += B) {
                // Имитируем загрузку блока из внешней памяти
                for (int i = 0; i < B; i++) {
                    for (int k = 0; k < B; k++) {
                        blockA[i * B + k] = (ii + i) + (kk + k); // A[ii+i][kk+k]
                        blockB[k * B + (jj + 0)] = (kk + k) * 0.1 + (jj); // B[kk+k][jj]
                    }
                }

                // Умножение блоков: blockC += blockA × blockB
                for (int i = 0; i < B; i++) {
                    for (int j = 0; j < B; j++) {
                        for (int k = 0; k < B; k++) {
                            blockC[i * B + j] += blockA[i * B + k] * blockB[k * B + j];
                        }
                    }
                }
            }

            // Запись blockC в результат
            for (int i = 0; i < B; i++) {
                for (int j = 0; j < B; j++) {
                    result[(ii + i) * N + (jj + j)] = blockC[i * B + j];
                }
            }
        }
    }

    free(result);
}
