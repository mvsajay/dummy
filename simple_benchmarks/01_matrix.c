/*
 * 01_matrix.c — Integer matrix multiply (32x32, 128x128 naive/transposed, add/sub)
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 01_matrix.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 01_matrix.elf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- 32x32 ---- */
#define MAT_S 32
typedef struct { int data[MAT_S][MAT_S]; } MatrixS;

static void mats_fill(MatrixS *m, int seed) {
    int i, j;
    for (i = 0; i < MAT_S; i++)
        for (j = 0; j < MAT_S; j++)
            m->data[i][j] = (seed + i * MAT_S + j) % 97;
}
static void mats_multiply(const MatrixS *a, const MatrixS *b, MatrixS *c) {
    int i, j, k;
    for (i = 0; i < MAT_S; i++)
        for (j = 0; j < MAT_S; j++) {
            int sum = 0;
            for (k = 0; k < MAT_S; k++) sum += a->data[i][k] * b->data[k][j];
            c->data[i][j] = sum;
        }
}
static int mats_checksum(const MatrixS *m) {
    int s = 0, i, j;
    for (i = 0; i < MAT_S; i++) for (j = 0; j < MAT_S; j++) s ^= m->data[i][j];
    return s;
}

/* ---- 128x128 ---- */
#define MAT_M 128
typedef struct { int data[MAT_M][MAT_M]; } MatrixM;

static void matm_fill(MatrixM *m, int seed) {
    int i, j;
    for (i = 0; i < MAT_M; i++)
        for (j = 0; j < MAT_M; j++)
            m->data[i][j] = (seed + i * MAT_M + j) % 97;
}
static void matm_multiply_naive(const MatrixM *a, const MatrixM *b, MatrixM *c) {
    int i, j, k;
    for (i = 0; i < MAT_M; i++)
        for (j = 0; j < MAT_M; j++) {
            int sum = 0;
            for (k = 0; k < MAT_M; k++) sum += a->data[i][k] * b->data[k][j];
            c->data[i][j] = sum;
        }
}
static void matm_transpose(const MatrixM *src, MatrixM *dst) {
    int i, j;
    for (i = 0; i < MAT_M; i++) for (j = 0; j < MAT_M; j++) dst->data[j][i] = src->data[i][j];
}
static void matm_multiply_transposed(const MatrixM *a, const MatrixM *bt, MatrixM *c) {
    int i, j, k;
    for (i = 0; i < MAT_M; i++)
        for (j = 0; j < MAT_M; j++) {
            int sum = 0;
            for (k = 0; k < MAT_M; k++) sum += a->data[i][k] * bt->data[j][k];
            c->data[i][j] = sum;
        }
}
static void matm_add(const MatrixM *a, const MatrixM *b, MatrixM *c) {
    int i, j;
    for (i = 0; i < MAT_M; i++) for (j = 0; j < MAT_M; j++) c->data[i][j] = a->data[i][j] + b->data[i][j];
}
static void matm_sub(const MatrixM *a, const MatrixM *b, MatrixM *c) {
    int i, j;
    for (i = 0; i < MAT_M; i++) for (j = 0; j < MAT_M; j++) c->data[i][j] = a->data[i][j] - b->data[i][j];
}
static int matm_checksum(const MatrixM *m) {
    int s = 0, i, j;
    for (i = 0; i < MAT_M; i++) for (j = 0; j < MAT_M; j++) s ^= m->data[i][j];
    return s;
}
static int matm_equal(const MatrixM *a, const MatrixM *b) {
    int i, j;
    for (i = 0; i < MAT_M; i++) for (j = 0; j < MAT_M; j++) if (a->data[i][j] != b->data[i][j]) return 0;
    return 1;
}

int main(void) {
    printf("=== [1a] Matrix Multiply (%dx%d integers, naive) ===\n", MAT_S, MAT_S);
    {
        MatrixS *a = (MatrixS *)malloc(sizeof(MatrixS));
        MatrixS *b = (MatrixS *)malloc(sizeof(MatrixS));
        MatrixS *c = (MatrixS *)malloc(sizeof(MatrixS));
        if (!a || !b || !c) { printf("malloc failed\n"); return 1; }
        mats_fill(a, 1); mats_fill(b, 7); mats_multiply(a, b, c);
        printf("  Checksum: %d\n", mats_checksum(c));
        printf("  C[0][0]=%d  C[%d][%d]=%d\n", c->data[0][0], MAT_S-1, MAT_S-1, c->data[MAT_S-1][MAT_S-1]);
        free(a); free(b); free(c);
    }
    printf("  DONE\n\n");

    printf("=== [1b] Matrix Multiply (%dx%d integers, naive) ===\n", MAT_M, MAT_M);
    {
        MatrixM *a  = (MatrixM *)malloc(sizeof(MatrixM));
        MatrixM *b  = (MatrixM *)malloc(sizeof(MatrixM));
        MatrixM *c1 = (MatrixM *)malloc(sizeof(MatrixM));
        MatrixM *bt = (MatrixM *)malloc(sizeof(MatrixM));
        MatrixM *c2 = (MatrixM *)malloc(sizeof(MatrixM));
        MatrixM *add_r = (MatrixM *)malloc(sizeof(MatrixM));
        MatrixM *sub_r = (MatrixM *)malloc(sizeof(MatrixM));
        if (!a || !b || !c1 || !bt || !c2 || !add_r || !sub_r) { printf("malloc failed\n"); return 1; }
        matm_fill(a, 3); matm_fill(b, 11);
        matm_multiply_naive(a, b, c1);
        printf("  Naive checksum    : %d\n", matm_checksum(c1));
        printf("  C1[0][0]=%d  C1[%d][%d]=%d\n", c1->data[0][0], MAT_M-1, MAT_M-1, c1->data[MAT_M-1][MAT_M-1]);
        printf("=== [1c] Matrix Multiply (%dx%d integers, transpose B) ===\n", MAT_M, MAT_M);
        matm_transpose(b, bt);
        matm_multiply_transposed(a, bt, c2);
        printf("  Transposed checksum: %d\n", matm_checksum(c2));
        printf("  Results identical: %s\n", matm_equal(c1, c2) ? "YES" : "NO");
        printf("=== [1d] Matrix Add / Subtract (%dx%d) ===\n", MAT_M, MAT_M);
        matm_add(c1, c2, add_r);
        matm_sub(add_r, c2, sub_r);
        printf("  Add checksum : %d\n", matm_checksum(add_r));
        printf("  (A+B)-B == A : %s\n", matm_equal(sub_r, c1) ? "YES" : "NO");
        free(a); free(b); free(c1); free(bt); free(c2); free(add_r); free(sub_r);
    }
    printf("  DONE\n");
    return 0;
}

// Made with Bob
