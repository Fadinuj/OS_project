#ifndef MAXCLIQUE_H
#define MAXCLIQUE_H

#include "graph.h"

/**
 * @file maxclique.h
 * Maximum Clique Algorithm implementation using backtracking
 * This module provides functionality to find the maximum clique in an undirected graph.
 * A clique is a subset of vertices where every pair of vertices is connected by an edge.
 * The maximum clique is the largest such subset.
 */

/**
 * Max Clique Result structure
 */
typedef struct {
    int* vertices;     // Array of vertices in the max clique
    int size;          // Size of the max clique
    int is_valid;      // 1 if result is valid, 0 otherwise
} MaxClique_Result;

/**
 * Find maximum clique using backtracking algorithm.
 * 
 * @param g Graph pointer
 * @param result OUT: Max clique result structure
 * @return 1 on success, 0 on failure
 */
int graph_max_clique(const Graph* g, MaxClique_Result* result);

/**
 * Print max clique result in a formatted way.
 * 
 * @param g Graph pointer
 */
void graph_print_max_clique(const Graph* g);

/**
 * Free max clique result memory.
 * 
 * @param result Max clique result structure to free
 */
void maxclique_result_free(MaxClique_Result* result);

/**
 * Get max clique size only.
 * @param g Graph pointer
 * @param clique_size OUT: pointer to store clique size
 * @return 1 on success, 0 on failure
 */
int graph_max_clique_size(const Graph* g, int* clique_size);

/**
 * Check if a given set of vertices forms a clique.
 * @param g Graph pointer
 * @param vertices Array of vertex indices
 * @param size Number of vertices to check
 * @return 1 if vertices form a clique, 0 otherwise
 */
int graph_is_clique(const Graph* g, int* vertices, int size);

/**
 * Find all maximal cliques (Bron-Kerbosch algorithm).
 * @param g Graph pointer
 * @param max_cliques OUT: array of cliques found
 * @param num_cliques OUT: number of cliques found
 * @return 1 on success, 0 on failure
 */
int graph_find_all_maximal_cliques(const Graph* g, MaxClique_Result** max_cliques, int* num_cliques);

#endif /* MAXCLIQUE_H */