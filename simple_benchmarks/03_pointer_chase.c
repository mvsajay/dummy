/*
 * 03_pointer_chase.c — 4096-node random-permutation heap pointer chase
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 03_pointer_chase.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 03_pointer_chase.elf
 */

#include <stdio.h>
#include <stdlib.h>

#define CHASE_N   4096
#define CHASE_PAD 7

typedef struct ChaseNode {
    struct ChaseNode *next;
    int               index;
    int               pad[CHASE_PAD];
} ChaseNode;

int main(void) {
    printf("=== [3] Pointer Chase (%d nodes, random permutation) ===\n", CHASE_N);

    ChaseNode **nodes = (ChaseNode **)malloc(CHASE_N * sizeof(ChaseNode *));
    if (!nodes) { printf("malloc failed\n"); return 1; }

    int i;
    for (i = 0; i < CHASE_N; i++) {
        nodes[i] = (ChaseNode *)malloc(sizeof(ChaseNode));
        if (!nodes[i]) { printf("malloc failed\n"); return 1; }
        nodes[i]->index = i;
        nodes[i]->next  = NULL;
        int p;
        for (p = 0; p < CHASE_PAD; p++) nodes[i]->pad[p] = i * CHASE_PAD + p;
    }

    int *perm = (int *)malloc(CHASE_N * sizeof(int));
    if (!perm) { printf("malloc failed\n"); return 1; }
    for (i = 0; i < CHASE_N; i++) perm[i] = i;

    unsigned int rng = 0xBEEFCAFEu;
    for (i = CHASE_N - 1; i > 0; i--) {
        rng = rng * 1664525u + 1013904223u;
        int j = (int)((rng >> 8) % (unsigned int)(i + 1));
        int tmp = perm[i]; perm[i] = perm[j]; perm[j] = tmp;
    }

    for (i = 0; i < CHASE_N - 1; i++) nodes[perm[i]]->next = nodes[perm[i+1]];
    nodes[perm[CHASE_N-1]]->next = nodes[perm[0]];

    /* Chase 1: full cycle */
    ChaseNode *cur = nodes[perm[0]];
    long long isum = 0, psum = 0;
    for (i = 0; i < CHASE_N; i++) {
        isum += cur->index;
        psum += cur->pad[0];
        cur   = cur->next;
    }
    long long exp_isum = (long long)CHASE_N * (CHASE_N - 1) / 2;
    long long exp_psum = (long long)CHASE_PAD * exp_isum;
    printf("  Full cycle index  sum: %lld  (expect %lld) %s\n", isum, exp_isum, isum==exp_isum?"OK":"FAIL");
    printf("  Full cycle pad[0] sum: %lld  (expect %lld) %s\n", psum, exp_psum, psum==exp_psum?"OK":"FAIL");

    /* Chase 2: write-back */
    cur = nodes[perm[0]];
    for (i = 0; i < CHASE_N/2; i++) { cur->pad[0] = cur->index * 2; cur = cur->next; }
    cur = nodes[perm[0]];
    int mismatches = 0;
    for (i = 0; i < CHASE_N/2; i++) { if (cur->pad[0] != cur->index*2) mismatches++; cur = cur->next; }
    printf("  Write-through chase mismatches: %d (expect 0)\n", mismatches);

    /* Chase 3: depth walk */
    cur = nodes[perm[0]];
    for (i = 0; i < CHASE_N/4; i++) cur = cur->next;
    printf("  Node at depth %d: index=%d (deterministic)\n", CHASE_N/4, cur->index);

    free(perm);
    for (i = 0; i < CHASE_N; i++) free(nodes[i]);
    free(nodes);
    printf("  DONE\n");
    return 0;
}

// Made with Bob
