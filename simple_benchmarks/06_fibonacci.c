/*
 * 06_fibonacci.c — Fibonacci iterative table + recursive memoised (N=40)
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 06_fibonacci.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 06_fibonacci.elf
 */

#include <stdio.h>

#define FIB_MAX 40

static long long fib_memo[FIB_MAX + 1];

static void fib_build_table(void) {
    fib_memo[0] = 0; fib_memo[1] = 1;
    int i;
    for (i = 2; i <= FIB_MAX; i++) fib_memo[i] = fib_memo[i-1] + fib_memo[i-2];
}

static long long fib_recursive(int n) {
    if (n <= 1) return n;
    if (fib_memo[n]) return fib_memo[n];
    fib_memo[n] = fib_recursive(n-1) + fib_recursive(n-2);
    return fib_memo[n];
}

int main(void) {
    printf("=== [6] Fibonacci (iterative table + recursive memoised, N=%d) ===\n", FIB_MAX);

    fib_build_table();
    printf("  fib(%d) iterative  = %lld\n", FIB_MAX, fib_memo[FIB_MAX]);

    int i;
    for (i = 2; i <= FIB_MAX; i++) fib_memo[i] = 0;
    long long result = fib_recursive(FIB_MAX);
    printf("  fib(%d) recursive  = %lld\n", FIB_MAX, result);
    printf("  Match: %s\n", (result == fib_memo[FIB_MAX]) ? "YES" : "NO");
    printf("  DONE\n");
    return 0;
}

// Made with Bob
