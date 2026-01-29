#ifndef MST_H
#define MST_H

#include "graph.h"

/**
 * @file mst.h
 * Minimum Spanning Tree Algorithm implementation using Prim's algorithm
 * 
 * This module provides functionality to find the minimum spanning tree (MST)
 * of a weighted graph using Prim's algorithm with a priority queue.
 * For unweighted graphs, all edges are treated as having weight 1.
 */

/**
 * Edge structure for MST representation
 */
typedef struct {
    int u, v;      // Edge endpoints
    int weight;    // Edge weight
} MST_Edge;

/**
 * MST Result structure
 */
typedef struct {
    MST_Edge* edges;  // Array of MST edges
    int num_edges;    // Number of edges in MST
    int total_weight; // Total weight of MST
    int is_connected; // 1 if graph is connected, 0 otherwise
} MST_Result;

/**
 * Calculate minimum spanning tree using Prim's algorithm.
 * For unweighted graphs, all edges have weight 1.
 * 
 * @param g Graph pointer
 * @param result OUT: MST result structure
 * @return 1 on success, 0 on failure
 */
int graph_mst_prim(const Graph* g, MST_Result* result);

/**
 * Print MST result in a formatted way.
 * 
 * @param g Graph pointer
 */
void graph_print_mst(const Graph* g);

/**
 * Free MST result memory.
 * 
 * @param result MST result structure to free
 */
void mst_result_free(MST_Result* result);

/**
 * Get MST total weight only (simpler interface).
 * 
 * @param g Graph pointer
 * @param total_weight OUT: pointer to store total weight
 * @return 1 on success, 0 on failure
 */
int graph_mst_weight(const Graph* g, int* total_weight);

#endif /* MST_H */