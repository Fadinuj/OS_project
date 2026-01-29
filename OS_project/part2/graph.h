#ifndef GRAPH_H
#define GRAPH_H

/**
 * @file graph.h
 *  Undirected, unweighted graph using adjacency lists (simple-graph policy).
 * Vertices are 0..n-1. Each undirected edge u--v is stored as two adjacency
 * nodes (u->v) and (v->u). Duplicate parallel edges are disallowed and at most
 * one self-loop per vertex is allowed.
 */



typedef struct EdgeNode {
    int to; //  Neighbor vertex index.
    struct EdgeNode* next; // Next node in the adjacency list.
} EdgeNode;

typedef struct {
    EdgeNode* head; // Head of the singly-linked list of neighbors
} Vertex;

typedef struct Graph {
    int n;  // Number of vertices (must be > 0).  
    Vertex* adj; // Array of adjacency lists of length n
} Graph;

/**
 * Create a graph with n vertices and no edges.
 * @param n Number of vertices (must be > 0).
 * @return Pointer to new graph, or NULL on invalid n / allocation failure.
 */

Graph* graph_create(int n);

/**
 * Free all memory of the graph (safe to call with NULL).
 * @param g Graph pointer (may be NULL).
 */
void   graph_destroy(Graph* g);

/**
 * Add an undirected edge u--v (simple-graph policy).
 *  If u!=v: rejects duplicates (only one u--v allowed).
 *  If u==v: allows at most one self-loop (internally two u->u entries).
 * @param g Graph pointer (non-NULL).
 * @param u Vertex index in [0,n-1].
 * @param v Vertex index in [0,n-1].
 * @return 0 on success; -1 out of bounds; -2 out of memory; -3 duplicate edge.
 */
int    graph_add_edge(Graph* g, int u, int v);

/**
 * Print adjacency lists to stdout. One line per vertex.
 * @param g Graph pointer (NULL is ignored).
 */
void   graph_print(const Graph* g);

/**
 * Check Euler circuit existence.
 * @return 1 if the graph (ignoring isolated vertices) is connected, all degrees are even, and there is at least one edge; otherwise 0.
 */
int    graph_has_euler_circuit(const Graph* g); /* 1 אם קיים, אחרת 0 */

/**
 * Find an Euler circuit using Hierholzer's algorithm.
 * @param g Graph pointer.
 * @param out_cycle On success, allocated array of vertex indices (caller frees).
 * @param out_len   On success, length of @p out_cycle (should be m+1).
 * @return 1 on success, 0 if no Euler circuit or on failure.
 */
int    graph_find_euler_circuit(const Graph* g, int** out_cycle, int* out_len);

#endif /* GRAPH_H */
