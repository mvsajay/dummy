/*
 * 05_hash_table.c — Open-addressing hash table (linear probe, int->int, cap=512)
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 05_hash_table.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 05_hash_table.elf
 */

#include <stdio.h>
#include <stdlib.h>

#define HT_CAP   512
#define HT_EMPTY -1

typedef struct { int key; int value; } HEntry;
typedef struct { HEntry *buckets; int cap; int count; } HashTable;

static HashTable *ht_create(int cap) {
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (!ht) { printf("malloc failed\n"); exit(1); }
    ht->buckets = (HEntry *)malloc(cap * sizeof(HEntry));
    if (!ht->buckets) { printf("malloc failed\n"); exit(1); }
    ht->cap = cap; ht->count = 0;
    int i;
    for (i = 0; i < cap; i++) ht->buckets[i].key = HT_EMPTY;
    return ht;
}

static void ht_insert(HashTable *ht, int key, int value) {
    int idx = (int)((unsigned int)(key * 2654435761u) & (unsigned int)(ht->cap - 1));
    while (ht->buckets[idx].key != HT_EMPTY && ht->buckets[idx].key != key)
        idx = (idx + 1) & (ht->cap - 1);
    if (ht->buckets[idx].key == HT_EMPTY) ht->count++;
    ht->buckets[idx].key   = key;
    ht->buckets[idx].value = value;
}

static int ht_lookup(HashTable *ht, int key, int *out) {
    int idx = (int)((unsigned int)(key * 2654435761u) & (unsigned int)(ht->cap - 1));
    int probes = 0;
    while (ht->buckets[idx].key != HT_EMPTY && probes < ht->cap) {
        if (ht->buckets[idx].key == key) { *out = ht->buckets[idx].value; return 1; }
        idx = (idx + 1) & (ht->cap - 1); probes++;
    }
    return 0;
}

static void ht_destroy(HashTable *ht) { free(ht->buckets); free(ht); }

int main(void) {
    printf("=== [5] Hash Table (open addressing, cap=%d) ===\n", HT_CAP);

    HashTable *ht = ht_create(HT_CAP);
    int i, val;

    for (i = 0; i < 200; i++) ht_insert(ht, i * 3 + 7, i * i);
    printf("  Entries inserted: %d\n", ht->count);

    int hits = 0;
    for (i = 0; i < 200; i++) if (ht_lookup(ht, i * 3 + 7, &val) && val == i * i) hits++;
    printf("  Lookup hits (expect 200): %d\n", hits);

    int misses = 0;
    for (i = 500; i < 700; i++) if (!ht_lookup(ht, i, &val)) misses++;
    printf("  Lookup misses (expect 200): %d\n", misses);

    ht_lookup(ht, 7, &val);  printf("  ht[7]=%d (expect 0)\n", val);
    ht_lookup(ht, 10, &val); printf("  ht[10]=%d (expect 1)\n", val);

    ht_destroy(ht);
    printf("  DONE\n");
    return 0;
}

// Made with Bob
