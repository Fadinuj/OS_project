#ifndef CLIQUE_COUNT_H
#define CLIQUE_COUNT_H

#include "graph.h"

/**
 * @file clique_count.h
 * Clique Counting Algorithm implementation
 * 
 * This module provides functionality to count the number of cliques of various sizes
 * in an undirected graph. Supports counting cliques of specific size or all cliques.
 */

/**
 * Clique Count Result structure
 */
typedef struct {
    int* counts_by_size;   // Array where counts_by_size[k] = number of k-cliques
    int max_size;          // Maximum clique size found
    int total_cliques;     // Total number of cliques (of all sizes >= 1)
    int is_valid;          // 1 if result is valid, 0 otherwise
} CliqueCount_Result;

/**
 * Count all cliques in the graph.
 * 
 * @param g Graph pointer
 * @param result OUT: Clique count result structure
 * @return 1 on success, 0 on failure
 */
int graph_count_all_cliques(const Graph* g, CliqueCount_Result* result);

/**
 * Count cliques of a specific size.
 * 
 * @param g Graph pointer
 * @param clique_size Size of cliques to count
 * @param count OUT: pointer to store the count
 * @return 1 on success, 0 on failure
 */
int graph_count_cliques_of_size(const Graph* g, int clique_size, int* count);

/**
 * Print clique count result in a formatted way.
 * 
 * @param g Graph pointer
 */
void graph_print_clique_counts(const Graph* g);

/**
 * Free clique count result memory.
 * 
 * @param result Clique count result structure to free
 */
void clique_count_result_free(CliqueCount_Result* result);

/**
 * Count triangles (3-cliques) in the graph - optimized version.
 * 
 * @param g Graph pointer
 * @param triangle_count OUT: pointer to store triangle count
 * @return 1 on success, 0 on failure
 */
int graph_count_triangles(const Graph* g, int* triangle_count);

/**
 * Count edges (2-cliques) in the graph.
 * 
 * @param g Graph pointer
 * @param edge_count OUT: pointer to store edge count
 * @return 1 on success, 0 on failure
 */
int graph_count_edges(const Graph* g, int* edge_count);

/**
 * Get total number of cliques of all sizes.
 * 
 * @param g Graph pointer
 * @param total_count OUT: pointer to store total count
 * @return 1 on success, 0 on failure
 */
int graph_total_clique_count(const Graph* g, int* total_count);

/**
 * Check if the graph has any cliques of a given size.
 * 
 * @param g Graph pointer
 * @param clique_size Size to check for
 * @return 1 if cliques of this size exist, 0 otherwise
 */
int graph_has_cliques_of_size(const Graph* g, int clique_size);

#endif /* CLIQUE_COUNT_H */