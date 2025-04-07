#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "memory_manager.h"
#include "variable_manager.h"

void block_multiply(int start_row, int end_row, int n, int m, int p, Meta mat_a, Meta mat_b, Meta mat_c) {
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < p; j++) {
            double sum = 0;
            for (int k = 0; k < m; k++) {
                double a_elem, b_elem;
                GET_MATRIX_ELEMENT(mat_a, i, k, m, &a_elem);
                GET_MATRIX_ELEMENT(mat_b, k, j, p, &b_elem);
                sum += a_elem * b_elem;
            }
            SET_MATRIX_ELEMENT(mat_c, i, j, p, sum);
        }
    }
}

int main() {
    init_memory();

    // Размеры матриц
    int n = 64, m = 64, p = 64; // Матрица A(n x m), B(m x p), C(n x p)

    // Определяем матрицы
    DEFINE_MATRIX(A, double, n, m);
    DEFINE_MATRIX(B, double, m, p);
    DEFINE_MATRIX(C, double, n, p);

    // Инициализируем матрицы A и B
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            SET_MATRIX_ELEMENT(A_meta, i, j, m, (double)(i + j));

    for (int i = 0; i < m; i++)
        for (int j = 0; j < p; j++)
            SET_MATRIX_ELEMENT(B_meta, i, j, p, (double)(i - j));

    // Создаем процессы для блочного умножения
    int num_processes = 1; // Два процесса
    int rows_per_process = n / num_processes;

    for (int i = 0; i < num_processes; i++) {
        int start_row = i * rows_per_process;
        int end_row = (i == num_processes - 1) ? n : start_row + rows_per_process;

        pid_t pid = fork();
        if (pid == 0) { // Ребенок
            block_multiply(start_row, end_row, n, m, p, A_meta, B_meta, C_meta);
            cleanup_memory();
            exit(0);
        }
    }

    // Ожидаем завершения дочерних процессов
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    // Вывод результата матрицы C
    printf("Matrix C:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < p; j++) {
            double elem;
            GET_MATRIX_ELEMENT(C_meta, i, j, p, &elem);
            printf("%.2f ", elem);
        }
        printf("\n");
    }

    cleanup_memory();
    return 0;
}
