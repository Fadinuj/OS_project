#include "maxclique.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Build adjacency matrix from adjacency list for efficient clique checking.
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
 * Backtracking algorithm to find maximum clique.
 */
static void max_clique_backtrack(int** adj_matrix, int n, int start_vertex,
                                int* current_clique, int current_size,
                                int* best_clique, int* best_size) {
    
    // Update best clique if current is larger
    if (current_size > *best_size) {
        *best_size = current_size;
        for (int i = 0; i < current_size; i++) {
            best_clique[i] = current_clique[i];
        }
    }
    
    // Try adding each remaining vertex
    for (int v = start_vertex; v < n; v++) {
        // Check if v is connected to all vertices in current clique
        if (is_connected_to_all(adj_matrix, v, current_clique, current_size)) {
            // Add v to current clique
            current_clique[current_size] = v;
            
            // Recursive call
            max_clique_backtrack(adj_matrix, n, v + 1, 
                               current_clique, current_size + 1,
                               best_clique, best_size);
        }
    }
}

/**
 * Find maximum clique using backtracking algorithm.
 */
int graph_max_clique(const Graph* g, MaxClique_Result* result) {
    if (!g || !result) return 0;
    
    int n = g->n;
    
    // Initialize result
    result->vertices = NULL;
    result->size = 0;
    result->is_valid = 0;
    
    if (n == 0) {
        result->is_valid = 1;
        return 1;
    }
    
    // Single vertex is always a clique of size 1
    if (n == 1) {
        result->vertices = (int*)malloc(sizeof(int));
        if (!result->vertices) return 0;
        result->vertices[0] = 0;
        result->size = 1;
        result->is_valid = 1;
        return 1;
    }
    
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
    
    // Allocate working arrays
    int* current_clique = (int*)malloc(n * sizeof(int));
    int* best_clique = (int*)malloc(n * sizeof(int));
    
    if (!current_clique || !best_clique) {
        free(current_clique); free(best_clique);
        for (int i = 0; i < n; i++) free(adj_matrix[i]);
        free(adj_matrix);
        return 0;
    }
    
    int best_size = 0;
    
    // Try starting from each vertex
    for (int start = 0; start < n; start++) {
        current_clique[0] = start;
        max_clique_backtrack(adj_matrix, n, start + 1,
                           current_clique, 1,
                           best_clique, &best_size);
    }
    
    // Store result
    if (best_size > 0) {
        result->vertices = (int*)malloc(best_size * sizeof(int));
        if (!result->vertices) {
            free(current_clique); free(best_clique);
            for (int i = 0; i < n; i++) free(adj_matrix[i]);
            free(adj_matrix);
            return 0;
        }
        
        for (int i = 0; i < best_size; i++) {
            result->vertices[i] = best_clique[i];
        }
        result->size = best_size;
        result->is_valid = 1;
    }
    
    // Cleanup
    free(current_clique); free(best_clique);
    for (int i = 0; i < n; i++) free(adj_matrix[i]);
    free(adj_matrix);
    
    return 1;
}

/**
 * Print max clique result in a formatted way.
 */
void graph_print_max_clique(const Graph* g) {
    if (!g) {
        printf("Error: NULL graph\n");
        return;
    }
    
    MaxClique_Result result;
    if (!graph_max_clique(g, &result)) {
        printf("Error: Failed to calculate max clique\n");
        return;
    }
    
    if (!result.is_valid || result.size == 0) {
        printf("No clique found\n");
        maxclique_result_free(&result);
        return;
    }
    
    printf("Maximum Clique:\n");
    printf("Size: %d\n", result.size);
    printf("Vertices: {");
    for (int i = 0; i < result.size; i++) {
        printf("%d", result.vertices[i]);
        if (i < result.size - 1) printf(", ");
    }
    printf("}\n");
    
    // Verify it's actually a clique
    if (graph_is_clique(g, result.vertices, result.size)) {
        printf("✓ Verified: This is a valid clique\n");
    } else {
        printf("✗ Error: This is not a valid clique!\n");
    }
    
    maxclique_result_free(&result);
}

/**
 * Free max clique result memory.
 */
void maxclique_result_free(MaxClique_Result* result) {
    if (result && result->vertices) {
        free(result->vertices);
        result->vertices = NULL;
        result->size = 0;
        result->is_valid = 0;
    }
}

/**
 * Get max clique size only (simpler interface).
 */
int graph_max_clique_size(const Graph* g, int* clique_size) {
    if (!g || !clique_size) return 0;
    
    MaxClique_Result result;
    if (!graph_max_clique(g, &result)) {
        return 0;
    }
    
    if (!result.is_valid) {
        maxclique_result_free(&result);
        return 0;
    }
    
    *clique_size = result.size;
    maxclique_result_free(&result);
    return 1;
}

/**
 * Check if a given set of vertices forms a clique.
 */
int graph_is_clique(const Graph* g, int* vertices, int size) {
    if (!g || !vertices || size < 0) return 0;
    
    if (size <= 1) return 1; // Single vertex or empty set is trivially a clique
    
    int n = g->n;
    
    // Check if all vertices are valid
    for (int i = 0; i < size; i++) {
        if (vertices[i] < 0 || vertices[i] >= n) return 0;
    }
    
    // Build adjacency matrix
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
    
    build_adjacency_matrix(g, adj_matrix);
    
    // Check if every pair of vertices is connected
    int is_clique = 1;
    for (int i = 0; i < size && is_clique; i++) {
        for (int j = i + 1; j < size && is_clique; j++) {
            if (!adj_matrix[vertices[i]][vertices[j]]) {
                is_clique = 0;
            }
        }
    }
    
    // Cleanup
    for (int i = 0; i < n; i++) free(adj_matrix[i]);
    free(adj_matrix);
    
    return is_clique;
}

/**
 * Bron-Kerbosch algorithm for finding all maximal cliques (simplified version).
 * Note: This is a basic implementation. For large graphs, optimizations are needed.
 */
static void bron_kerbosch(int** adj_matrix, int n,
                         int* R, int R_size,          // Current clique
                         int* P, int P_size,          // Candidate vertices
                         int* X, int X_size,          // Excluded vertices
                         MaxClique_Result** results, int* num_results, int* capacity) {
    
    if (P_size == 0 && X_size == 0) {
        // Found a maximal clique
        if (*num_results >= *capacity) {
            *capacity *= 2;
            *results = (MaxClique_Result*)realloc(*results, *capacity * sizeof(MaxClique_Result));
            if (!*results) return;
        }
        
        (*results)[*num_results].vertices = (int*)malloc(R_size * sizeof(int));
        if (!(*results)[*num_results].vertices) return;
        
        for (int i = 0; i < R_size; i++) {
            (*results)[*num_results].vertices[i] = R[i];
        }
        (*results)[*num_results].size = R_size;
        (*results)[*num_results].is_valid = 1;
        (*num_results)++;
        return;
    }
    
    // Make copies of P for iteration
    int* P_copy = (int*)malloc(P_size * sizeof(int));
    if (!P_copy) return;
    for (int i = 0; i < P_size; i++) P_copy[i] = P[i];
    int P_copy_size = P_size;
    
    for (int i = 0; i < P_copy_size; i++) {
        int v = P_copy[i];
        
        // R' = R ∪ {v}
        R[R_size] = v;
        
        // P' = P ∩ N(v)
        int* P_new = (int*)malloc(n * sizeof(int));
        int P_new_size = 0;
        for (int j = 0; j < P_size; j++) {
            if (adj_matrix[v][P[j]]) {
                P_new[P_new_size++] = P[j];
            }
        }
        
        // X' = X ∩ N(v)
        int* X_new = (int*)malloc(n * sizeof(int));
        int X_new_size = 0;
        for (int j = 0; j < X_size; j++) {
            if (adj_matrix[v][X[j]]) {
                X_new[X_new_size++] = X[j];
            }
        }
        
        // Recursive call
        bron_kerbosch(adj_matrix, n, R, R_size + 1, P_new, P_new_size, X_new, X_new_size,
                     results, num_results, capacity);
        
        // Move v from P to X
        for (int j = 0; j < P_size; j++) {
            if (P[j] == v) {
                for (int k = j; k < P_size - 1; k++) {
                    P[k] = P[k + 1];
                }
                P_size--;
                break;
            }
        }
        X[X_size++] = v;
        
        free(P_new);
        free(X_new);
    }
    
    free(P_copy);
}

/**
 * Find all maximal cliques using Bron-Kerbosch algorithm.
 */
int graph_find_all_maximal_cliques(const Graph* g, MaxClique_Result** max_cliques, int* num_cliques) {
    if (!g || !max_cliques || !num_cliques) return 0;
    
    int n = g->n;
    *max_cliques = NULL;
    *num_cliques = 0;
    
    if (n == 0) return 1;
    
    // Build adjacency matrix
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
    
    build_adjacency_matrix(g, adj_matrix);
    
    // Initialize for Bron-Kerbosch
    int* R = (int*)malloc(n * sizeof(int)); // Current clique (empty)
    int* P = (int*)malloc(n * sizeof(int)); // All vertices initially
    int* X = (int*)malloc(n * sizeof(int)); // Excluded vertices (empty)
    
    if (!R || !P || !X) {
        free(R); free(P); free(X);
        for (int i = 0; i < n; i++) free(adj_matrix[i]);
        free(adj_matrix);
        return 0;
    }
    
    // Initialize P with all vertices
    for (int i = 0; i < n; i++) {
        P[i] = i;
    }
    
    int capacity = 10;
    *max_cliques = (MaxClique_Result*)malloc(capacity * sizeof(MaxClique_Result));
    if (!*max_cliques) {
        free(R); free(P); free(X);
        for (int i = 0; i < n; i++) free(adj_matrix[i]);
        free(adj_matrix);
        return 0;
    }
    
    bron_kerbosch(adj_matrix, n, R, 0, P, n, X, 0, max_cliques, num_cliques, &capacity);
    
    // Cleanup
    free(R); free(P); free(X);
    for (int i = 0; i < n; i++) free(adj_matrix[i]);
    free(adj_matrix);
    
    return 1;
}