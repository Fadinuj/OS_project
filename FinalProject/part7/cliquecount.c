#include "cliquecount.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Build adjacency matrix from adjacency list for efficient clique operations.
 */
static int build_adjacency_matrix(const Graph* g, int** adj_matrix) {
    int n = g->n;
    
    // Initialize matrix to 0
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            adj_matrix[i][j] = 0;
        }
    }
    
    // Fill adjacency matrix
    for (int u = 0; u < n; u++) {
        for (EdgeNode* edge = g->adj[u].head; edge; edge = edge->next) {
            int v = edge->to;
            if (u != v) { // Skip self-loops
                adj_matrix[u][v] = 1;
            }
        }
    }
    
    return 1;
}

/**
 * Check if vertex v is connected to all vertices in current clique.
 */
static int is_connected_to_all(int** adj_matrix, int v, int* current_clique, int clique_size) {
    for (int i = 0; i < clique_size; i++) {
        if (!adj_matrix[v][current_clique[i]]) {
            return 0;
        }
    }
    return 1;
}

/**
 * Recursive backtracking to count cliques of all sizes.
 */
static void count_cliques_recursive(int** adj_matrix, int n, int start_vertex,
                                   int* current_clique, int current_size,
                                   int* counts_by_size, int max_possible_size) {
    
    // Count current clique if size >= 1
    if (current_size > 0 && current_size <= max_possible_size) {
        counts_by_size[current_size]++;
    }
    
    // Try adding each remaining vertex
    for (int v = start_vertex; v < n; v++) {
        // Check if v is connected to all vertices in current clique
        if (is_connected_to_all(adj_matrix, v, current_clique, current_size)) {
            // Add v to current clique
            current_clique[current_size] = v;
            
            // Recursive call
            count_cliques_recursive(adj_matrix, n, v + 1, 
                                   current_clique, current_size + 1,
                                   counts_by_size, max_possible_size);
        }
    }
}

/**
 * Recursive backtracking to count cliques of specific size only.
 */
static void count_cliques_of_size_recursive(int** adj_matrix, int n, int start_vertex,
                                           int* current_clique, int current_size,
                                           int target_size, int* count) {
    
    // Found a clique of target size
    if (current_size == target_size) {
        (*count)++;
        return;
    }
    
    // Pruning: if we can't reach target size with remaining vertices
    if (current_size + (n - start_vertex) < target_size) {
        return;
    }
    
    // Try adding each remaining vertex
    for (int v = start_vertex; v < n; v++) {
        // Check if v is connected to all vertices in current clique
        if (is_connected_to_all(adj_matrix, v, current_clique, current_size)) {
            // Add v to current clique
            current_clique[current_size] = v;
            
            // Recursive call
            count_cliques_of_size_recursive(adj_matrix, n, v + 1, 
                                           current_clique, current_size + 1,
                                           target_size, count);
        }
    }
}

/**
 * Count all cliques in the graph.
 */
int graph_count_all_cliques(const Graph* g, CliqueCount_Result* result) {
    if (!g || !result) return 0;
    
    int n = g->n;
    
    // Initialize result
    result->counts_by_size = NULL;
    result->max_size = 0;
    result->total_cliques = 0;
    result->is_valid = 0;
    
    if (n == 0) {
        result->is_valid = 1;
        return 1;
    }
    
    // Allocate counts array (index 0 unused, indices 1 to n for clique sizes)
    result->counts_by_size = (int*)calloc(n + 1, sizeof(int));
    if (!result->counts_by_size) return 0;
    
    // Allocate adjacency matrix
    int** adj_matrix = (int**)malloc(n * sizeof(int*));
    if (!adj_matrix) {
        free(result->counts_by_size);
        return 0;
    }
    
    for (int i = 0; i < n; i++) {
        adj_matrix[i] = (int*)malloc(n * sizeof(int));
        if (!adj_matrix[i]) {
            for (int j = 0; j < i; j++) free(adj_matrix[j]);
            free(adj_matrix);
            free(result->counts_by_size);
            return 0;
        }
    }
    
    // Build adjacency matrix
    build_adjacency_matrix(g, adj_matrix);
    
    // Allocate working array
    int* current_clique = (int*)malloc(n * sizeof(int));
    if (!current_clique) {
        for (int i = 0; i < n; i++) free(adj_matrix[i]);
        free(adj_matrix);
        free(result->counts_by_size);
        return 0;
    }
    
    // Count cliques starting from each vertex
    count_cliques_recursive(adj_matrix, n, 0, current_clique, 0, result->counts_by_size, n);
    
    // Calculate total and find max size
    int total = 0;
    int max_size = 0;
    for (int i = 1; i <= n; i++) {
        if (result->counts_by_size[i] > 0) {
            total += result->counts_by_size[i];
            max_size = i;
        }
    }
    
    result->total_cliques = total;
    result->max_size = max_size;
    result->is_valid = 1;
    
    // Cleanup
    free(current_clique);
    for (int i = 0; i < n; i++) free(adj_matrix[i]);
    free(adj_matrix);
    
    return 1;
}

/**
 * Count cliques of a specific size.
 */
int graph_count_cliques_of_size(const Graph* g, int clique_size, int* count) {
    if (!g || !count || clique_size < 1) return 0;
    
    int n = g->n;
    *count = 0;
    
    if (clique_size > n) return 1; // No cliques larger than number of vertices
    
    // Allocate adjacency matrix
    int** adj_matrix = (int**)malloc(n * sizeof(int*));
    if (!adj_matrix) return 0;
    
    for (int i = 0; i < n; i++) {
        adj_matrix[i] = (int*)malloc(n * sizeof(int));
        if (!adj_matrix[i]) {
            for (int j = 0; j < i; j++) free(adj_matrix[j]);
            free(adj_matrix);
            return 0;
        }
    }
    
    // Build adjacency matrix
    build_adjacency_matrix(g, adj_matrix);
    
    // Allocate working array
    int* current_clique = (int*)malloc(n * sizeof(int));
    if (!current_clique) {
        for (int i = 0; i < n; i++) free(adj_matrix[i]);
        free(adj_matrix);
        return 0;
    }
    
    // Count cliques of specific size
    count_cliques_of_size_recursive(adj_matrix, n, 0, current_clique, 0, clique_size, count);
    
    // Cleanup
    free(current_clique);
    for (int i = 0; i < n; i++) free(adj_matrix[i]);
    free(adj_matrix);
    
    return 1;
}

/**
 * Print clique count result in a formatted way.
 */
void graph_print_clique_counts(const Graph* g) {
    if (!g) {
        printf("Error: NULL graph\n");
        return;
    }
    
    CliqueCount_Result result;
    if (!graph_count_all_cliques(g, &result)) {
        printf("Error: Failed to count cliques\n");
        return;
    }
    
    if (!result.is_valid) {
        printf("Invalid result\n");
        clique_count_result_free(&result);
        return;
    }
    
    printf("Clique Count Analysis:\n");
    printf("Total cliques: %d\n", result.total_cliques);
    printf("Maximum clique size: %d\n", result.max_size);
    printf("\nBreakdown by size:\n");
    
    for (int i = 1; i <= result.max_size; i++) {
        if (result.counts_by_size[i] > 0) {
            printf("  Size %d: %d cliques\n", i, result.counts_by_size[i]);
        }
    }
    
    if (result.total_cliques == 0) {
        printf("  No cliques found (isolated vertices only)\n");
    }
    
    clique_count_result_free(&result);
}

/**
 * Free clique count result memory.
 */
void clique_count_result_free(CliqueCount_Result* result) {
    if (result && result->counts_by_size) {
        free(result->counts_by_size);
        result->counts_by_size = NULL;
        result->max_size = 0;
        result->total_cliques = 0;
        result->is_valid = 0;
    }
}

/**
 * Count triangles (3-cliques) in the graph - optimized version.
 */
int graph_count_triangles(const Graph* g, int* triangle_count) {
    if (!g || !triangle_count) return 0;
    
    int n = g->n;
    *triangle_count = 0;
    
    if (n < 3) return 1; // Need at least 3 vertices for triangle
    
    // Allocate adjacency matrix
    int** adj_matrix = (int**)malloc(n * sizeof(int*));
    if (!adj_matrix) return 0;
    
    for (int i = 0; i < n; i++) {
        adj_matrix[i] = (int*)malloc(n * sizeof(int));
        if (!adj_matrix[i]) {
            for (int j = 0; j < i; j++) free(adj_matrix[j]);
            free(adj_matrix);
            return 0;
        }
    }
    
    // Build adjacency matrix
    build_adjacency_matrix(g, adj_matrix);
    
    // Count triangles: for each triple (i,j,k) where i < j < k
    int count = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (adj_matrix[i][j]) {
                for (int k = j + 1; k < n; k++) {
                    if (adj_matrix[i][k] && adj_matrix[j][k]) {
                        count++;
                    }
                }
            }
        }
    }
    
    *triangle_count = count;
    
    // Cleanup
    for (int i = 0; i < n; i++) free(adj_matrix[i]);
    free(adj_matrix);
    
    return 1;
}

/**
 * Count edges (2-cliques) in the graph.
 */
int graph_count_edges(const Graph* g, int* edge_count) {
    if (!g || !edge_count) return 0;
    
    *edge_count = 0;
    
    // Count edges by traversing adjacency lists
    for (int u = 0; u < g->n; u++) {
        for (EdgeNode* edge = g->adj[u].head; edge; edge = edge->next) {
            int v = edge->to;
            if (u < v) { // Count each undirected edge only once
                (*edge_count)++;
            }
        }
    }
    
    return 1;
}

/**
 * Get total number of cliques of all sizes.
 */
int graph_total_clique_count(const Graph* g, int* total_count) {
    if (!g || !total_count) return 0;
    
    CliqueCount_Result result;
    if (!graph_count_all_cliques(g, &result)) {
        return 0;
    }
    
    if (!result.is_valid) {
        clique_count_result_free(&result);
        return 0;
    }
    
    *total_count = result.total_cliques;
    clique_count_result_free(&result);
    return 1;
}

/**
 * Check if the graph has any cliques of a given size.
 */
int graph_has_cliques_of_size(const Graph* g, int clique_size) {
    int count;
    if (!graph_count_cliques_of_size(g, clique_size, &count)) {
        return 0;
    }
    return count > 0;
}