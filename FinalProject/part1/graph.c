#include "graph.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Implementation of an undirected, unweighted graph using adjacency lists.
 */
static int in_bounds(const Graph* g, int v) {
    return (g != NULL && v >= 0 && v < g->n);
}

/* Count how many times v appears in u's adjacency list */
static int count_neighbor(const Graph* g, int u, int v){
    int c = 0;
    for (EdgeNode* e = g->adj[u].head; e; e = e->next)
        if (e->to == v) c++;
    return c;
}

/* Check if the undirected edge (u--v) already exists according to our simple-graph policy */
static int edge_exists_simple(const Graph* g, int u, int v){
    if (u == v) {
        /* One undirected self-loop u—u is stored as TWO u->u entries.
           If we already have two, we don't allow another. */
        return count_neighbor(g, u, u) >= 2;
    } else {
        /* For non-loops, having one u->v entry means u--v already exists */
        return count_neighbor(g, u, v) >= 1;
    }
}


/**
 *Create a new graph with n vertices (0..n-1) and no edges.
 * Initializes all adjacency lists to empty.
 * @param n Number of vertices. Must be > 0.
 * @return Pointer to a newly allocated Graph on success.
 */
Graph* graph_create(int n) {
    if (n <= 0) return NULL;

    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (!g) return NULL;

    g->n = n;
    g->adj = (Vertex*)calloc((size_t)n, sizeof(Vertex));
    if (!g->adj) { free(g); return NULL; }

    return g;
}
/**
 * Destroy a graph and free all associated memory.
 * Frees every adjacency node in every list, then the arrays and the graph.
 * @param g Pointer to the graph to destroy (may be NULL).
 */
void graph_destroy(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->n; ++i) {
        EdgeNode* cur = g->adj[i].head;
        while (cur) {
            EdgeNode* tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }
    free(g->adj);
    free(g);
}

/**
 * Add an undirected edge between vertices  u and  v.
 * This function inserts two adjacency nodes: (u -> v) and (v -> u).
 * The graph permits:
 * Self-loops (u == v): two entries (u -> u) are inserted.
 * Allocation is done before linking the nodes into the lists to avoid
 * leaving a half-inserted edge if the second allocation fails.
 * @param g Pointer to the graph (must not be NULL).
 * @param u Source vertex index in [0, n-1].
 * @param v Destination vertex index in [0, n-1].
 * @return 0 on success;
 *         -1 if  u or v is out of bounds;
 *         -2 if memory allocation fails (no modification is made).
 */
int graph_add_edge(Graph* g, int u, int v) {
    if (!in_bounds(g, u) || !in_bounds(g, v)) return -1;

    /* simple-graph policy: block duplicates / >1 self-loop */
    if (edge_exists_simple(g, u, v)) return -3;

    /* allocate both nodes BEFORE linking to avoid half-insert on OOM */
    EdgeNode* e1 = (EdgeNode*)malloc(sizeof(EdgeNode));
    EdgeNode* e2 = (EdgeNode*)malloc(sizeof(EdgeNode));
    if (!e1 || !e2) { free(e1); free(e2); return -2; }

    e1->to = v; e1->next = g->adj[u].head;
    e2->to = u; e2->next = g->adj[v].head;

    g->adj[u].head = e1;
    g->adj[v].head = e2;

    return 0;
}

/**
 * Print the graph's adjacency lists to stdout.
 * The format is one line per vertex:  <vertex>: <neighbor> <neighbor> ... 
 * @param g Pointer to the graph (NULL is ignored).
 */
void graph_print(const Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->n; ++i) {
        printf("%d:", i);
        EdgeNode* cur = g->adj[i].head;
        while (cur) {
            printf(" %d", cur->to);
            cur = cur->next;
        }
        printf("\n");
    }
}
