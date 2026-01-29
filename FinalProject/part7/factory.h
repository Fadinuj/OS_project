#ifndef ALGORITHM_FACTORY_H
#define ALGORITHM_FACTORY_H

#include "graph.h"
#include "algorithm_strategy.h"

typedef enum {
    ALGO_EULER = 1,
    ALGO_MAX_FLOW,
    ALGO_MST,
    ALGO_MAX_CLIQUE,
    ALGO_CLIQUE_COUNT
} AlgorithmType;

/**
 * Factory Pattern - Creates Strategy objects based on algorithm type
 */

/**
 * Create strategy instance using Factory pattern.
 * @param algo_type Algorithm type to create
 * @return Strategy pointer, or NULL if algorithm not supported
 */
AlgorithmStrategy* algorithm_factory_create_strategy(AlgorithmType algo_type);

/**
 * Get algorithm type from algorithm ID.
 * @param algorithm_id Algorithm ID (1-5)
 * @return Algorithm type enum
 */
AlgorithmType algorithm_factory_get_type(int algorithm_id);

/**
 * Factory method to execute algorithm using both Factory and Strategy patterns.
 * @param g Graph pointer
 * @param algorithm_id Algorithm ID
 * @return Result string (caller must free), or NULL on failure
 */
char* algorithm_factory_execute(const Graph* g, int algorithm_id);

/**
 * Print available algorithms that the factory can create.
 */
void algorithm_factory_print_available(void);

/**
 * Check if algorithm type is supported by factory.
 */
int algorithm_factory_is_supported(AlgorithmType algo_type);

#endif /* ALGORITHM_FACTORY_H */