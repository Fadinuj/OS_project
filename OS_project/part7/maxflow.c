#include "maxflow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/**
 * Queue implementation for BFS
 */
typedef struct {
    int* data;
    int front, rear, size, capacity;
} Queue;

static Queue* queue_create(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) return NULL;
    
    q->data = (int*)malloc(sizeof(int) * capacity);
    if (!q->data) {
        free(q);
        return NULL;
    }
    
    q->front = q->rear = q->size = 0;
    q->capacity = capacity;
    return q;
}

static void queue_destroy(Queue* q) {
    if (q) {
        free(q->data);
        free(q);
    }
}

static int queue_is_empty(const Queue* q) {
    return q->size == 0;
}

static int queue_enqueue(Queue* q, int item) {
    if (q->size >= q->capacity) return 0; // Queue full
    
    q->data[q->rear] = item;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    return 1;
}

static int queue_dequeue(Queue* q) {
    if (queue_is_empty(q)) return -1;
    
    int item = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return item;
}

/**
 * Convert adjacency list graph to capacity matrix.
 * For unweighted graph, each edge has capacity 1.
 * 
 * @param g Graph pointer
 * @param capacity_matrix OUT: n×n matrix to fill with capacities
 * @return 1 on success, 0 on failure
 */
static int build_capacity_matrix(const Graph* g, int** capacity_matrix) {
    int n = g->n;
    
    // Initialize matrix to 0
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            capacity_matrix[i][j] = 0;
        }
    }
    
    // Fill capacity matrix from adjacency lists using actual edge weights
    for (int u = 0; u < n; u++) {
        for (EdgeNode* edge = g->adj[u].head; edge; edge = edge->next) {
            int v = edge->to;
            if (u != v) { // Skip self-loops for flow networks
                // Use edge weight as capacity (instead of hardcoded 1)
                capacity_matrix[u][v] = edge->weight;
            }
        }
    }
    return 1;
}

/**
 * BFS to find augmenting path in residual graph.
 * 
 * @param res_graph Residual graph as capacity matrix
 * @param n Number of vertices
 * @param source Source vertex
 * @param sink Sink vertex
 * @param parent OUT: array to store the path
 * @return 1 if augmenting path found, 0 otherwise
 */
static int bfs_find_path(int** res_graph, int n, int source, int sink, int* parent) {
    int* visited = (int*)calloc(n, sizeof(int));
    if (!visited) return 0;
    
    Queue* q = queue_create(n);
    if (!q) {
        free(visited);
        return 0;
    }
    
    queue_enqueue(q, source);
    visited[source] = 1;
    parent[source] = -1;
    
    int found = 0;
    
    while (!queue_is_empty(q) && !found) {
        int u = queue_dequeue(q);
        
        for (int v = 0; v < n; v++) {
            if (!visited[v] && res_graph[u][v] > 0) {
                parent[v] = u;
                visited[v] = 1;
                queue_enqueue(q, v);
                
                if (v == sink) {
                    found = 1;
                    break;
                }
            }
        }
    }
    
    queue_destroy(q);
    free(visited);
    return found;
}

/**
 * Find minimum value in augmenting path.
 */
static int find_path_flow(int** res_graph, int source, int sink, int* parent) {
    int path_flow = INT_MAX;
    
    for (int v = sink; v != source; v = parent[v]) {
        int u = parent[v];
        if (res_graph[u][v] < path_flow) {
            path_flow = res_graph[u][v];
        }
    }
    
    return path_flow;
}

/**
 * Update residual graph along the augmenting path.
 */
static void update_residual_graph(int** res_graph, int source, int sink, int* parent, int path_flow) {
    for (int v = sink; v != source; v = parent[v]) {
        int u = parent[v];
        res_graph[u][v] -= path_flow;  // Forward edge
        res_graph[v][u] += path_flow;  // Backward edge
    }
}

/**
 * Calculate maximum flow from source to sink using Edmonds-Karp algorithm.
 */
int graph_max_flow(const Graph* g, int source, int sink, int* max_flow_value) {
    if (!g || !max_flow_value || source < 0 || sink < 0 || 
        source >= g->n || sink >= g->n || source == sink) {
        return 0;
    }
    
    int n = g->n;
    *max_flow_value = 0;
    
    // Allocate capacity/residual matrix
    int** res_graph = (int**)malloc(n * sizeof(int*));
    if (!res_graph) return 0;
    
    for (int i = 0; i < n; i++) {
        res_graph[i] = (int*)malloc(n * sizeof(int));
        if (!res_graph[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                free(res_graph[j]);
            }
            free(res_graph);
            return 0;
        }
    }
    
    // Build initial capacity matrix
    if (!build_capacity_matrix(g, res_graph)) {
        // Cleanup
        for (int i = 0; i < n; i++) {
            free(res_graph[i]);
        }
        free(res_graph);
        return 0;
    }
    
    int* parent = (int*)malloc(n * sizeof(int));
    if (!parent) {
        for (int i = 0; i < n; i++) {
            free(res_graph[i]);
        }
        free(res_graph);
        return 0;
    }
    
    int max_flow = 0;
    
    // Edmonds-Karp main loop
    while (bfs_find_path(res_graph, n, source, sink, parent)) {
        // Find minimum capacity along the path
        int path_flow = find_path_flow(res_graph, source, sink, parent);
        
        // Update residual graph
        update_residual_graph(res_graph, source, sink, parent, path_flow);
        
        // Add path flow to total flow
        max_flow += path_flow;
    }
    
    *max_flow_value = max_flow;
    
    // Cleanup
    free(parent);
    for (int i = 0; i < n; i++) {
        free(res_graph[i]);
    }
    free(res_graph);
    
    return 1;
}

/**
 * Calculate maximum flow with default source=0 and sink=n-1.
 */
int graph_max_flow_default(const Graph* g, int* max_flow_value) {
    if (!g || g->n < 2) return 0;
    return graph_max_flow(g, 0, g->n - 1, max_flow_value);
}

/**
 * Print maximum flow result in a formatted string.
 */
void graph_print_max_flow(const Graph* g, int source, int sink) {
    if (!g) {
        printf("Error: NULL graph\n");
        return;
    }
    
    if (source < 0 || sink < 0 || source >= g->n || sink >= g->n) {
        printf("Error: Invalid source (%d) or sink (%d) for graph with %d vertices\n", 
               source, sink, g->n);
        return;
    }
    
    if (source == sink) {
        printf("Error: Source and sink cannot be the same vertex\n");
        return;
    }
    
    int max_flow_value;
    if (graph_max_flow(g, source, sink, &max_flow_value)) {
        printf("Max flow from vertex %d to vertex %d is: %d\n", 
               source, sink, max_flow_value);
    } else {
        printf("Failed to calculate max flow from vertex %d to vertex %d\n", 
               source, sink);
    }
}