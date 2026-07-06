/*
 * 02_large_matrix.c — 256x256 cache-blocked (tiled) matrix multiply
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 02_large_matrix.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 02_large_matrix.elf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAT_L  256
#define TILE   32

typedef struct { int *data; } MatrixL;

#define ML(m,i,j) ((m)->data[(i)*MAT_L+(j)])

static MatrixL *matl_alloc(void) {
    MatrixL *m = (MatrixL *)malloc(sizeof(MatrixL));
    if (!m) { printf("malloc failed\n"); exit(1); }
    m->data = (int *)malloc(MAT_L * MAT_L * sizeof(int));
    if (!m->data) { printf("malloc failed\n"); exit(1); }
    return m;
}
static void matl_free(MatrixL *m) { free(m->data); free(m); }

static void matl_fill(MatrixL *m, int seed) {
    int i, j;
    for (i = 0; i < MAT_L; i++)
        for (j = 0; j < MAT_L; j++)
            ML(m, i, j) = (seed + i * 13 + j * 7) % 97;
}

static void matl_multiply_naive(const MatrixL *a, const MatrixL *b, MatrixL *c) {
    int i, j, k;
    memset(c->data, 0, MAT_L * MAT_L * sizeof(int));
    for (i = 0; i < MAT_L; i++)
        for (j = 0; j < MAT_L; j++) {
            int sum = 0;
            for (k = 0; k < MAT_L; k++) sum += ML(a, i, k) * ML(b, k, j);
            ML(c, i, j) = sum;
        }
}

static void matl_multiply_tiled(const MatrixL *a, const MatrixL *b, MatrixL *c) {
    int i, j, k, ii, jj, kk;
    memset(c->data, 0, MAT_L * MAT_L * sizeof(int));
    for (ii = 0; ii < MAT_L; ii += TILE)
        for (jj = 0; jj < MAT_L; jj += TILE)
            for (kk = 0; kk < MAT_L; kk += TILE)
                for (i = ii; i < ii + TILE; i++)
                    for (k = kk; k < kk + TILE; k++) {
                        int aik = ML(a, i, k);
                        for (j = jj; j < jj + TILE; j++)
                            ML(c, i, j) += aik * ML(b, k, j);
                    }
}

static int matl_checksum(const MatrixL *m) {
    int s = 0, i, j;
    for (i = 0; i < MAT_L; i++) for (j = 0; j < MAT_L; j++) s ^= ML(m, i, j);
    return s;
}
static int matl_equal(const MatrixL *a, const MatrixL *b) {
    return memcmp(a->data, b->data, MAT_L * MAT_L * sizeof(int)) == 0;
}

int main(void) {
    printf("=== [2] Large Matrix Multiply (%dx%d, naive + tiled, tile=%d) ===\n",
           MAT_L, MAT_L, TILE);

    MatrixL *a      = matl_alloc();
    MatrixL *b      = matl_alloc();
    MatrixL *c_ref  = matl_alloc();
    MatrixL *c_til  = matl_alloc();
    MatrixL *d      = matl_alloc();
    MatrixL *c_til2 = matl_alloc();

    matl_fill(a, 5);
    matl_fill(b, 13);

    matl_multiply_naive(a, b, c_ref);
    printf("  Naive  checksum: %d\n", matl_checksum(c_ref));
    printf("  C_ref[0][0]=%d  C_ref[%d][%d]=%d\n",
           ML(c_ref, 0, 0), MAT_L-1, MAT_L-1, ML(c_ref, MAT_L-1, MAT_L-1));

    matl_multiply_tiled(a, b, c_til);
    printf("  Tiled  checksum: %d\n", matl_checksum(c_til));
    printf("  Naive == Tiled : %s\n", matl_equal(c_ref, c_til) ? "YES" : "NO");

    matl_fill(d, 19);
    matl_multiply_tiled(c_til, d, c_til2);
    printf("  Chain C*D tiled checksum: %d\n", matl_checksum(c_til2));

    matl_free(a); matl_free(b); matl_free(c_ref);
    matl_free(c_til); matl_free(d); matl_free(c_til2);
    printf("  DONE\n");
    return 0;
}

// Made with Bob
