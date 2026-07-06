/*
 * 04_merge_sort.c — Recursive merge sort on 4096 integers
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 04_merge_sort.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 04_merge_sort.elf
 */

#include <stdio.h>
#include <stdlib.h>

#define SORT_N 4096

static void merge(int *arr, int *tmp, int lo, int mid, int hi) {
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) tmp[k++] = (arr[i] <= arr[j]) ? arr[i++] : arr[j++];
    while (i < mid) tmp[k++] = arr[i++];
    while (j < hi)  tmp[k++] = arr[j++];
    for (i = lo; i < hi; i++) arr[i] = tmp[i];
}

static void merge_sort(int *arr, int *tmp, int lo, int hi) {
    if (hi - lo < 2) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort(arr, tmp, lo, mid);
    merge_sort(arr, tmp, mid, hi);
    merge(arr, tmp, lo, mid, hi);
}

static int is_sorted(int *arr, int n) {
    int i;
    for (i = 1; i < n; i++) if (arr[i] < arr[i-1]) return 0;
    return 1;
}

int main(void) {
    printf("=== [4] Merge Sort (%d integers) ===\n", SORT_N);

    int *arr = (int *)malloc(SORT_N * sizeof(int));
    int *tmp = (int *)malloc(SORT_N * sizeof(int));
    if (!arr || !tmp) { printf("malloc failed\n"); return 1; }

    unsigned int x = 0xDEADBEEFu;
    int i;
    for (i = 0; i < SORT_N; i++) {
        x = x * 1664525u + 1013904223u;
        arr[i] = (int)(x >> 16) & 0x7FFF;
    }

    printf("  First five before: %d %d %d %d %d\n", arr[0], arr[1], arr[2], arr[3], arr[4]);
    merge_sort(arr, tmp, 0, SORT_N);
    printf("  First five after : %d %d %d %d %d\n", arr[0], arr[1], arr[2], arr[3], arr[4]);
    printf("  Sorted: %s\n", is_sorted(arr, SORT_N) ? "YES" : "NO");

    free(arr); free(tmp);
    printf("  DONE\n");
    return 0;
}

// Made with Bob
