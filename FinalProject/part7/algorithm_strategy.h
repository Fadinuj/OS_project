#ifndef ALGORITHM_STRATEGY_H
#define ALGORITHM_STRATEGY_H

#include "graph.h"

/**
 * @file algorithm_strategy.h
 * Strategy Pattern implementation for Graph Algorithms
 * 
 * This module implements the Strategy pattern to allow runtime selection
 * and execution of different graph algorithms with a unified interface.
 */

/**
 * Algorithm Strategy function pointer type
 */
typedef char* (*AlgorithmExecuteFunc)(const Graph* g);

/**
 * Algorithm Strategy structure
 */
typedef struct {
    AlgorithmExecuteFunc execute;  // Function pointer to algorithm execution
    const char* name;              // Algorithm name
    const char* description;       // Algorithm description
    int id;                       // Algorithm ID
} AlgorithmStrategy;

/**
 * Algorithm Context - manages strategy selection and execution
 */
typedef struct {
    AlgorithmStrategy* strategy;   // Current strategy
    const Graph* graph;           // Graph to operate on
} AlgorithmContext;

/**
 * Initialize algorithm context.
 * 
 * @param context Algorithm context to initialize
 * @param graph Graph to set for operations
 */
void algorithm_context_init(AlgorithmContext* context, const Graph* graph);

/**
 * Set algorithm strategy.
 * 
 * @param context Algorithm context
 * @param strategy Strategy to set
 */
void algorithm_context_set_strategy(AlgorithmContext* context, AlgorithmStrategy* strategy);

/**
 * Execute current algorithm strategy.
 * 
 * @param context Algorithm context
 * @return Result string (caller must free), or NULL on failure
 */
char* algorithm_context_execute(AlgorithmContext* context);

/**
 * Get strategy by algorithm ID.
 * 
 * @param algorithm_id Algorithm ID (1-5)
 * @return Strategy pointer, or NULL if not found
 */
AlgorithmStrategy* algorithm_get_strategy(int algorithm_id);

/**
 * Get strategy by algorithm name.
 * 
 * @param algorithm_name Algorithm name
 * @return Strategy pointer, or NULL if not found
 */
AlgorithmStrategy* algorithm_get_strategy_by_name(const char* algorithm_name);

/**
 * Get all available strategies.
 * 
 * @param count OUT: number of strategies returned
 * @return Array of strategy pointers
 */
AlgorithmStrategy** algorithm_get_all_strategies(int* count);

/**
 * Print all available strategies.
 */
void algorithm_print_strategies(void);

/**
 * Execute algorithm by ID with strategy pattern.
 * 
 * @param graph Graph to operate on
 * @param algorithm_id Algorithm ID
 * @return Result string (caller must free), or NULL on failure
 */
char* algorithm_execute_by_id(const Graph* graph, int algorithm_id);

#endif /* ALGORITHM_STRATEGY_H */