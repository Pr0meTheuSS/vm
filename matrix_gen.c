// generate_matrices.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 512

void generate_matrix(FILE* f, int size) {
    for (int i = 0; i < size * size; i++) {
        double val = (double)(rand() % 1000) / 100.0; // числа от 0.00 до 9.99
        fwrite(&val, sizeof(double), 1, f);
    }
}

int main() {
    srand(time(NULL));

    FILE* fa = fopen("matrixA.bin", "wb");
    FILE* fb = fopen("matrixB.bin", "wb");

    if (!fa || !fb) {
        perror("Failed to create matrix files");
        return 1;
    }

    printf("Generating %dx%d matrices...\n", MATRIX_SIZE, MATRIX_SIZE);
    generate_matrix(fa, MATRIX_SIZE);
    generate_matrix(fb, MATRIX_SIZE);

    fclose(fa);
    fclose(fb);
    printf("Done. Files: matrixA.bin, matrixB.bin\n");
    return 0;
}
