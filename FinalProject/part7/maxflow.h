#ifndef MAXFLOW_H
#define MAXFLOW_H

#include "graph.h"

/**
 * @file maxflow.h
 * Max Flow Algorithm implementation using Edmonds-Karp (BFS-based Ford-Fulkerson)
 * This module provides functionality to find the maximum flow in a flow network
 * from a source vertex to a sink vertex using the Edmonds-Karp algorithm.
 */

/**
 * Calculate maximum flow from source to sink using Edmonds-Karp algorithm.
 * 
 * @param g Graph pointer (treated as flow network with unit capacities)
 * @param source Source vertex index
 * @param sink Sink vertex index  
 * @param max_flow_value OUT: pointer to store the maximum flow value
 * @return 1 on success, 0 on failure (invalid input or no path exists)
 */
int graph_max_flow(const Graph* g, int source, int sink, int* max_flow_value);

/**
 * Calculate maximum flow with default source=0 and sink=n-1.
 * 
 * @param g Graph pointer
 * @param max_flow_value OUT: pointer to store the maximum flow value
 * @return 1 on success, 0 on failure
 */
int graph_max_flow_default(const Graph* g, int* max_flow_value);

/**
 * Print maximum flow result in a formatted string.
 * 
 * @param g Graph pointer
 * @param source Source vertex
 * @param sink Sink vertex
 */
void graph_print_max_flow(const Graph* g, int source, int sink);

#endif /* MAXFLOW_H */