/*
 * 07_string_manual.c — Manual byte-level string operations (no libc SIMD paths)
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 07_string_manual.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 07_string_manual.elf
 */

#include <stdio.h>

#define STR_LEN 256

static int my_strlen(const char *s) {
    int n = 0; while (s[n]) n++; return n;
}
static void my_strcpy(char *dst, const char *src) {
    while ((*dst++ = *src++) != '\0') {}
}
static int my_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}
static void str_reverse(char *s, int len) {
    int lo = 0, hi = len - 1;
    while (lo < hi) { char t = s[lo]; s[lo] = s[hi]; s[hi] = t; lo++; hi--; }
}
static int str_count_char(const char *s, char c) {
    int cnt = 0; while (*s) { if (*s == c) cnt++; s++; } return cnt;
}

int main(void) {
    printf("=== [7] String Operations (byte-level, len=%d) ===\n", STR_LEN);

    char src[STR_LEN], dst[STR_LEN], rev[STR_LEN];
    int i;

    for (i = 0; i < STR_LEN - 1; i++) src[i] = (char)('a' + (i % 26));
    src[STR_LEN - 1] = '\0';

    printf("  my_strlen: %d (expect %d)\n", my_strlen(src), STR_LEN - 1);
    my_strcpy(dst, src);
    printf("  my_strcmp after copy: %d (expect 0)\n", my_strcmp(src, dst));
    my_strcpy(rev, src);
    str_reverse(rev, my_strlen(rev));
    printf("  First 6 chars of reversed: %.6s\n", rev);
    printf("  Occurrences of 'a' in src: %d\n", str_count_char(src, 'a'));
    dst[0] = 'Z';
    printf("  my_strcmp after mutation: %d (expect >0)\n", my_strcmp(dst, src));

    printf("  DONE\n");
    return 0;
}

// Made with Bob
