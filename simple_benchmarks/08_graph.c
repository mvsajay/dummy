/*
 * 08_graph.c — Graph traversals: BFS, DFS, Dijkstra (V=512, E~4096)
 * Part of the simple_benchmarks suite for RISC-V -> POWER translation validation.
 *
 * Build:
 *   /opt/riscv64-rv64g/bin/gcc -mabi=lp64d -fPIE -static 08_graph.c \
 *       -B/opt/glibc-rv64g/lib -L/opt/glibc-rv64g/lib -o 08_graph.elf
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define G_NODES 512
#define G_EDGES 4096

typedef struct GEdge { int dst; int w; struct GEdge *next; } GEdge;
typedef struct { GEdge *head[G_NODES]; int n; } Graph;

static Graph *graph_alloc(void) {
    Graph *g = (Graph *)calloc(1, sizeof(Graph));
    if (!g) { printf("malloc failed\n"); exit(1); }
    g->n = G_NODES;
    return g;
}
static void graph_add_edge(Graph *g, int u, int v, int w) {
    GEdge *e = (GEdge *)malloc(sizeof(GEdge));
    if (!e) { printf("malloc failed\n"); exit(1); }
    e->dst = v; e->w = w; e->next = g->head[u]; g->head[u] = e;
}
static void graph_build(Graph *g) {
    unsigned int rng = 0xABCD1234u;
    int i;
    for (i = 0; i < G_EDGES; i++) {
        rng = rng * 1664525u + 1013904223u; int u = (rng >> 16) % G_NODES;
        rng = rng * 1664525u + 1013904223u; int v = (rng >> 16) % G_NODES;
        rng = rng * 1664525u + 1013904223u; int w = (int)((rng >> 16) % 99) + 1;
        if (u != v) graph_add_edge(g, u, v, w);
    }
    for (i = 0; i < G_NODES - 1; i++) graph_add_edge(g, i, i + 1, 1);
}
static void graph_free(Graph *g) {
    int i;
    for (i = 0; i < g->n; i++) { GEdge *e = g->head[i]; while (e) { GEdge *nx = e->next; free(e); e = nx; } }
    free(g);
}

/* BFS: hop distance src->dst */
static int bfs(const Graph *g, int src, int dst, int *dist_out) {
    int *dist  = (int *)malloc(g->n * sizeof(int));
    int *queue = (int *)malloc(g->n * sizeof(int));
    if (!dist || !queue) { printf("malloc failed\n"); exit(1); }
    int i; for (i = 0; i < g->n; i++) dist[i] = -1;
    dist[src] = 0;
    int head = 0, tail = 0;
    queue[tail++] = src;
    while (head < tail) {
        int u = queue[head++];
        GEdge *e = g->head[u];
        while (e) {
            if (dist[e->dst] == -1) {
                dist[e->dst] = dist[u] + 1;
                queue[tail++] = e->dst;
                if (e->dst == dst) goto bfs_done;
            }
            e = e->next;
        }
    }
bfs_done:;
    int result = dist[dst]; *dist_out = result;
    free(dist); free(queue);
    return result >= 0;
}

/* BFS: reachable vertex count */
static int bfs_reachable(const Graph *g, int src) {
    int *visited = (int *)calloc(g->n, sizeof(int));
    int *queue   = (int *)malloc(g->n * sizeof(int));
    if (!visited || !queue) { printf("malloc failed\n"); exit(1); }
    visited[src] = 1;
    int head = 0, tail = 0, count = 1;
    queue[tail++] = src;
    while (head < tail) {
        int u = queue[head++];
        GEdge *e = g->head[u];
        while (e) {
            if (!visited[e->dst]) { visited[e->dst] = 1; queue[tail++] = e->dst; count++; }
            e = e->next;
        }
    }
    free(visited); free(queue);
    return count;
}

/* DFS iterative: reachable vertex count */
static int dfs_reachable(const Graph *g, int src) {
    int *visited = (int *)calloc(g->n, sizeof(int));
    int *stack   = (int *)malloc(g->n * sizeof(int));
    if (!visited || !stack) { printf("malloc failed\n"); exit(1); }
    visited[src] = 1;
    int top = 0, count = 0;
    stack[top++] = src;
    while (top > 0) {
        int u = stack[--top]; count++;
        GEdge *e = g->head[u];
        while (e) {
            if (!visited[e->dst]) { visited[e->dst] = 1; stack[top++] = e->dst; }
            e = e->next;
        }
    }
    free(visited); free(stack);
    return count;
}

/* Dijkstra: shortest weighted path */
typedef struct { int node; int dist; } HeapEntry;
typedef struct { HeapEntry *data; int size; int cap; } MinHeap;

static MinHeap *heap_alloc(int cap) {
    MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
    h->data = (HeapEntry *)malloc(cap * sizeof(HeapEntry));
    if (!h || !h->data) { printf("malloc failed\n"); exit(1); }
    h->size = 0; h->cap = cap; return h;
}
static void heap_free(MinHeap *h) { free(h->data); free(h); }
static void heap_push(MinHeap *h, int node, int dist) {
    int i = h->size++;
    h->data[i].node = node; h->data[i].dist = dist;
    while (i > 0) {
        int p = (i-1)/2;
        if (h->data[p].dist <= h->data[i].dist) break;
        HeapEntry t = h->data[p]; h->data[p] = h->data[i]; h->data[i] = t; i = p;
    }
}
static HeapEntry heap_pop(MinHeap *h) {
    HeapEntry top = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (1) {
        int l=2*i+1, r=2*i+2, s=i;
        if (l < h->size && h->data[l].dist < h->data[s].dist) s=l;
        if (r < h->size && h->data[r].dist < h->data[s].dist) s=r;
        if (s==i) break;
        HeapEntry t = h->data[i]; h->data[i] = h->data[s]; h->data[s] = t; i = s;
    }
    return top;
}
static int dijkstra(const Graph *g, int src, int dst) {
    int *dist = (int *)malloc(g->n * sizeof(int));
    int i; for (i = 0; i < g->n; i++) dist[i] = INT_MAX;
    dist[src] = 0;
    MinHeap *h = heap_alloc(G_EDGES + G_NODES);
    heap_push(h, src, 0);
    while (h->size > 0) {
        HeapEntry cur = heap_pop(h);
        if (cur.dist > dist[cur.node]) continue;
        if (cur.node == dst) break;
        GEdge *e = g->head[cur.node];
        while (e) {
            int nd = dist[cur.node] + e->w;
            if (nd < dist[e->dst]) { dist[e->dst] = nd; heap_push(h, e->dst, nd); }
            e = e->next;
        }
    }
    int result = dist[dst];
    heap_free(h); free(dist);
    return result;
}

int main(void) {
    printf("=== [8] Graph Traversals (V=%d, E~%d) ===\n", G_NODES, G_EDGES);

    Graph *g = graph_alloc();
    graph_build(g);

    int hop_dist = 0;
    int reachable = bfs(g, 0, G_NODES-1, &hop_dist);
    printf("  BFS  0->%d: reachable=%s  hops=%d\n", G_NODES-1, reachable?"YES":"NO", hop_dist);

    int bfs_reach = bfs_reachable(g, 0);
    printf("  BFS  reachable from 0: %d / %d vertices\n", bfs_reach, G_NODES);

    int dfs_reach = dfs_reachable(g, 0);
    printf("  DFS  reachable from 0: %d / %d vertices\n", dfs_reach, G_NODES);
    printf("  BFS == DFS reachable : %s\n", bfs_reach == dfs_reach ? "YES" : "NO");

    int w_dist = dijkstra(g, 0, G_NODES-1);
    printf("  Dijkstra 0->%d: weight=%d\n", G_NODES-1, w_dist);

    int mid = G_NODES/2;
    int d1 = dijkstra(g, 0, mid);
    int d2 = dijkstra(g, mid, G_NODES-1);
    printf("  Triangle: d(0,%d)=%d + d(%d,%d)=%d >= d(0,%d)=%d : %s\n",
           mid, d1, mid, G_NODES-1, d2, G_NODES-1, w_dist,
           (d1+d2 >= w_dist) ? "OK" : "FAIL");

    graph_free(g);
    printf("  DONE\n");
    return 0;
}

// Made with Bob
