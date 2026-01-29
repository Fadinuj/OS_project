#ifndef GRAPH_H
#define GRAPH_H


/**
 * @file graph.h
 * Create/destroy a graph
 * Add an undirected edge
 * Print the adjacency lists
 * Vertices are indexed 0..n-1.
 * Memory allocation failures are reported via
 * negative return codes where applicable.
 */



 /**
 * Each node represents one directed incidence from a vertex to a neighbor.
 * For an undirected edge (u -- v), two nodes are created: (u -> v) and (v -> u).
 */
typedef struct EdgeNode {
    int to; //Neighbor vertex index 
    struct EdgeNode* next;// Next adjacency node in the list.
} EdgeNode;

 /**
  * Adjacency list head for a vertex.
*/
typedef struct {
    EdgeNode* head; //list of neighbors of this vertex.
} Vertex;


/**
 * Graph object: undirected, adjacency-list representation.
 */
typedef struct Graph {
    int n; // Number of vertices in the graph (must be > 0).      
    Vertex* adj; //
} Graph;


/**
 * Create a new graph with vertices (0..n-1) and no edges.
 */
Graph* graph_create(int n);


/**
 * Destroy a graph and free all associated memory.
 */
void   graph_destroy(Graph* g);

/**
 * Add an undirected edge between vertices  u and  v
 */
int    graph_add_edge(Graph* g, int u, int v);


/**
 * Print the graph's adjacency lists to stdout.
 */
void   graph_print(const Graph* g);

#endif /* GRAPH_H */
