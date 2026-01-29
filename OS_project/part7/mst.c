#include "mst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/**
 * Priority Queue implementation for Prim's algorithm
 * Min-heap storing {weight, vertex} pairs
 */
typedef struct {
    int weight;
    int vertex;
} PQ_Node;

typedef struct {
    PQ_Node* data;
    int size;
    int capacity;
} PriorityQueue;

static PriorityQueue* pq_create(int capacity) {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if (!pq) return NULL;
    
    pq->data = (PQ_Node*)malloc(sizeof(PQ_Node) * capacity);
    if (!pq->data) {
        free(pq);
        return NULL;
    }
    
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

static void pq_destroy(PriorityQueue* pq) {
    if (pq) {
        free(pq->data);
        free(pq);
    }
}

static void pq_swap(PQ_Node* a, PQ_Node* b) {
    PQ_Node temp = *a;
    *a = *b;
    *b = temp;
}

static void pq_heapify_up(PriorityQueue* pq, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (pq->data[index].weight >= pq->data[parent].weight) break;
        
        pq_swap(&pq->data[index], &pq->data[parent]);
        index = parent;
    }
}

static void pq_heapify_down(PriorityQueue* pq, int index) {
    while (1) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        
        if (left < pq->size && pq->data[left].weight < pq->data[smallest].weight) {
            smallest = left;
        }
        
        if (right < pq->size && pq->data[right].weight < pq->data[smallest].weight) {
            smallest = right;
        }
        
        if (smallest == index) break;
        
        pq_swap(&pq->data[index], &pq->data[smallest]);
        index = smallest;
    }
}

static int pq_push(PriorityQueue* pq, int weight, int vertex) {
    if (pq->size >= pq->capacity) return 0; // Queue full
    
    pq->data[pq->size].weight = weight;
    pq->data[pq->size].vertex = vertex;
    pq_heapify_up(pq, pq->size);
    pq->size++;
    return 1;
}

static PQ_Node pq_pop(PriorityQueue* pq) {
    PQ_Node result = pq->data[0];
    pq->size--;
    if (pq->size > 0) {
        pq->data[0] = pq->data[pq->size];
        pq_heapify_down(pq, 0);
    }
    return result;
}

static int pq_is_empty(const PriorityQueue* pq) {
    return pq->size == 0;
}

/**
 * Build weight matrix from adjacency list using actual edge weights.
 * Now supports real weights from the graph structure.
 */
static int build_weight_matrix(const Graph* g, int** weight_matrix) {
    int n = g->n;
    
    // Initialize matrix to 0 (no edge)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            weight_matrix[i][j] = 0;
        }
    }
    
    // Fill weight matrix from adjacency lists with actual weights
    for (int u = 0; u < n; u++) {
        for (EdgeNode* edge = g->adj[u].head; edge; edge = edge->next) {
            int v = edge->to;
            if (u != v) { // Skip self-loops
                weight_matrix[u][v] = edge->weight; // Use actual edge weight!
            }
        }
    }
    
    return 1;
}

/**
 * Calculate minimum spanning tree using Prim's algorithm.
 * Now properly handles weighted graphs.
 */
int graph_mst_prim(const Graph* g, MST_Result* result) {
    if (!g || !result || g->n < 1) return 0;
    
    int n = g->n;
    
    // Initialize result
    result->edges = NULL;
    result->num_edges = 0;
    result->total_weight = 0;
    result->is_connected = 0;
    
    if (n == 1) {
        result->is_connected = 1; // Single vertex is trivially connected
        return 1;
    }
    
    // Allocate weight matrix
    int** weight_matrix = (int**)malloc(n * sizeof(int*));
    if (!weight_matrix) return 0;
    
    for (int i = 0; i < n; i++) {
        weight_matrix[i] = (int*)malloc(n * sizeof(int));
        if (!weight_matrix[i]) {
            for (int j = 0; j < i; j++) free(weight_matrix[j]);
            free(weight_matrix);
            return 0;
        }
    }
    
    // Build weight matrix with actual edge weights
    build_weight_matrix(g, weight_matrix);
    
    // Prim's algorithm variables
    int* in_mst = (int*)calloc(n, sizeof(int));
    int* key = (int*)malloc(n * sizeof(int));
    int* parent = (int*)malloc(n * sizeof(int));
    
    if (!in_mst || !key || !parent) {
        free(in_mst); free(key); free(parent);
        for (int i = 0; i < n; i++) free(weight_matrix[i]);
        free(weight_matrix);
        return 0;
    }
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        key[i] = INT_MAX;
        parent[i] = -1;
    }
    key[0] = 0; // Start from vertex 0
    
    PriorityQueue* pq = pq_create(n * n); // Generous capacity
    if (!pq) {
        free(in_mst); free(key); free(parent);
        for (int i = 0; i < n; i++) free(weight_matrix[i]);
        free(weight_matrix);
        return 0;
    }
    
    pq_push(pq, 0, 0); // {weight=0, vertex=0}
    
    // Prim's main loop
    while (!pq_is_empty(pq)) {
        PQ_Node current = pq_pop(pq);
        int u = current.vertex;
        
        if (in_mst[u]) continue; // Already in MST
        
        in_mst[u] = 1;
        
        // Update keys of adjacent vertices using actual edge weights
        for (int v = 0; v < n; v++) {
            int weight = weight_matrix[u][v];
            if (weight > 0 && !in_mst[v] && weight < key[v]) {
                key[v] = weight;
                parent[v] = u;
                pq_push(pq, weight, v);
            }
        }
    }
    
    // Check if all vertices are reachable (graph is connected)
    int vertices_in_mst = 0;
    for (int i = 0; i < n; i++) {
        if (in_mst[i]) vertices_in_mst++;
    }
    
    if (vertices_in_mst != n) {
        // Graph is not connected
        result->is_connected = 0;
        pq_destroy(pq);
        free(in_mst); free(key); free(parent);
        for (int i = 0; i < n; i++) free(weight_matrix[i]);
        free(weight_matrix);
        return 1; // Success, but no spanning tree
    }
    
    result->is_connected = 1;
    
    // Build MST edges array
    result->edges = (MST_Edge*)malloc((n-1) * sizeof(MST_Edge));
    if (!result->edges) {
        pq_destroy(pq);
        free(in_mst); free(key); free(parent);
        for (int i = 0; i < n; i++) free(weight_matrix[i]);
        free(weight_matrix);
        return 0;
    }
    
    int edge_count = 0;
    int total_weight = 0;
    
    for (int v = 1; v < n; v++) { // Skip vertex 0 (root)
        if (parent[v] != -1) {
            result->edges[edge_count].u = parent[v];
            result->edges[edge_count].v = v;
            result->edges[edge_count].weight = weight_matrix[parent[v]][v];
            total_weight += weight_matrix[parent[v]][v];
            edge_count++;
        }
    }
    
    result->num_edges = edge_count;
    result->total_weight = total_weight;
    
    // Cleanup
    pq_destroy(pq);
    free(in_mst); free(key); free(parent);
    for (int i = 0; i < n; i++) free(weight_matrix[i]);
    free(weight_matrix);
    
    return 1;
}

/**
 * Print MST result in a formatted way.
 */
void graph_print_mst(const Graph* g) {
    if (!g) {
        printf("Error: NULL graph\n");
        return;
    }
    
    MST_Result result;
    if (!graph_mst_prim(g, &result)) {
        printf("Error: Failed to calculate MST\n");
        return;
    }
    
    if (!result.is_connected) {
        printf("Graph is not connected - no spanning tree exists\n");
        return;
    }
    
    printf("Minimum Spanning Tree:\n");
    printf("Total weight: %d\n", result.total_weight);
    printf("Edges in MST:\n");
    
    for (int i = 0; i < result.num_edges; i++) {
        printf("  %d -- %d (weight: %d)\n", 
               result.edges[i].u, result.edges[i].v, result.edges[i].weight);
    }
    
    mst_result_free(&result);
}

/**
 * Free MST result memory.
 */
void mst_result_free(MST_Result* result) {
    if (result && result->edges) {
        free(result->edges);
        result->edges = NULL;
        result->num_edges = 0;
    }
}

/**
 * Get MST total weight only (simpler interface).
 */
int graph_mst_weight(const Graph* g, int* total_weight) {
    if (!g || !total_weight) return 0;
    
    MST_Result result;
    if (!graph_mst_prim(g, &result)) {
        return 0;
    }
    
    if (!result.is_connected) {
        mst_result_free(&result);
        return 0;
    }
    
    *total_weight = result.total_weight;
    mst_result_free(&result);
    return 1;
}