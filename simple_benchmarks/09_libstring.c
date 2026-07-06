/*
 * 09_libstring.c — Standard libc string/memory function calls
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Exercises: strlen, strcpy, strncpy, strcat, strncat, strcmp, strncmp,
 *            strstr, strchr, strrchr, memcpy, memmove, memset, memcmp,
 *            strtol, strtoul, sprintf, sscanf
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 09_libstring.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 09_libstring.elf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LSTR_N 256

int main(void) {
    printf("=== [9] Library String Operations ===\n");

    static char a[LSTR_N], b[LSTR_N], c[LSTR_N], needle[16];
    int i;

    for (i = 0; i < LSTR_N - 1; i++) a[i] = (char)('a' + i % 26);
    a[LSTR_N - 1] = '\0';

    printf("  strlen(a)              = %zu  (expect %d)\n", strlen(a), LSTR_N - 1);

    strcpy(b, a);
    printf("  strcmp(a, strcpy b)    = %d   (expect 0)\n", strcmp(a, b));

    memset(c, 0xFF, sizeof(c));
    strncpy(c, a, 10); c[10] = '\0';
    printf("  strncpy(a,10)          = \"%.10s\"  (expect \"abcdefghij\")\n", c);

    strcpy(b, "hello"); strcat(b, "world");
    printf("  strcat                 = \"%s\"  (expect \"helloworld\")\n", b);
    strncat(b, "!!!XYZ", 3);
    printf("  strncat(...,3)         = \"%s\"  (expect \"helloworld!!!\")\n", b);

    printf("  strcmp(\"abc\",\"abd\")    = %d   (expect <0)\n", strcmp("abc", "abd"));
    printf("  strncmp(\"abcX\",\"abcY\",3)= %d  (expect 0)\n",  strncmp("abcX", "abcY", 3));

    strcpy(a, "the quick brown fox jumps over the lazy dog");
    const char *found    = strstr(a, "fox");
    const char *notfound = strstr(a, "cat");
    printf("  strstr(a,\"fox\")        = \"%s\"\n", found ? found : "(null)");
    printf("  strstr(a,\"cat\")        = %s\n",     notfound ? notfound : "(null)");

    const char *ch  = strchr(a, 'o');
    const char *rch = strrchr(a, 'o');
    printf("  strchr(a,'o')  offset  = %d  (first 'o')\n",  (int)(ch  - a));
    printf("  strrchr(a,'o') offset  = %d  (last  'o')\n",  (int)(rch - a));
    printf("  First != Last 'o'      : %s\n", ch != rch ? "YES" : "NO");

    static char src[64], dst[64];
    for (i = 0; i < 64; i++) src[i] = (char)(i + 1);
    memcpy(dst, src, 64);
    printf("  memcpy 64B identical   = %s  (expect YES)\n", memcmp(src, dst, 64) == 0 ? "YES" : "NO");

    static char mbuf[32];
    for (i = 0; i < 32; i++) mbuf[i] = (char)i;
    memmove(mbuf + 4, mbuf, 20);
    printf("  memmove overlap [4]    = %d  (expect 0)\n", (int)mbuf[4]);
    printf("  memmove overlap [7]    = %d  (expect 3)\n", (int)mbuf[7]);

    static char msbuf[16];
    memset(msbuf, 0xAB, sizeof(msbuf));
    int all_ab = 1;
    for (i = 0; i < 16; i++) if ((unsigned char)msbuf[i] != 0xAB) { all_ab = 0; break; }
    printf("  memset 0xAB all match  = %s  (expect YES)\n", all_ab ? "YES" : "NO");

    static char ca[8] = {1,2,3,4,5,6,7,8};
    static char cb[8] = {1,2,3,4,5,6,7,9};
    printf("  memcmp equal prefix    = %d  (expect 0 for first 7)\n", memcmp(ca, cb, 7));
    printf("  memcmp differ at [7]   = %d  (expect <0)\n",            memcmp(ca, cb, 8));

    char *endp;
    long lv = strtol("  -12345xyz", &endp, 10);
    printf("  strtol(\"-12345\")       = %ld  (expect -12345)\n", lv);
    printf("  strtol endp            = \"%s\"  (expect \"xyz\")\n", endp);

    unsigned long uv = strtoul("0xDEAD", NULL, 16);
    printf("  strtoul(\"0xDEAD\",16)   = %lu  (expect 57005)\n", uv);

    static char fmtbuf[64];
    int orig_a = 42, orig_b = -7;
    sprintf(fmtbuf, "vals:%d,%d", orig_a, orig_b);
    printf("  sprintf result         = \"%s\"\n", fmtbuf);
    int read_a = 0, read_b = 0;
    int nread = sscanf(fmtbuf, "vals:%d,%d", &read_a, &read_b);
    printf("  sscanf nread=%d a=%d b=%d  (expect 2 42 -7)\n", nread, read_a, read_b);

    strcpy(needle, "the");
    static const char *haystack = "the cat sat on the mat near the hat by the vat";
    const char *p = haystack;
    int occurrences = 0;
    while ((p = strstr(p, needle)) != NULL) { occurrences++; p++; }
    printf("  strstr count \"the\"     = %d  (expect 4)\n", occurrences);

    printf("  DONE\n");
    return 0;
}

// Made with Bob
