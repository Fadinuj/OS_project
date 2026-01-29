#include "factory.h"
#include "maxflow.h"
#include "mst.h"
#include "maxclique.h"
#include "cliquecount.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Factory Pattern Implementation
 * The Factory creates Strategy objects, demonstrating both patterns working together
 */

/**
 * Get algorithm type from algorithm ID.
 */
AlgorithmType algorithm_factory_get_type(int algorithm_id) {
    switch (algorithm_id) {
        case 1: return ALGO_EULER;
        case 2: return ALGO_MAX_FLOW;
        case 3: return ALGO_MST;
        case 4: return ALGO_MAX_CLIQUE;
        case 5: return ALGO_CLIQUE_COUNT;
        default: return -1; // Invalid
    }
}

/**
 * Factory method - Creates Strategy objects
 */
AlgorithmStrategy* algorithm_factory_create_strategy(AlgorithmType algo_type) {
    printf("Factory: Creating strategy for algorithm type %d\n", algo_type);
    
    switch (algo_type) {
        case ALGO_EULER:
            printf("Factory: Creating Euler Circuit Strategy\n");
            return algorithm_get_strategy(1);
            
        case ALGO_MAX_FLOW:
            printf("Factory: Creating Max Flow Strategy\n");
            return algorithm_get_strategy(2);
            
        case ALGO_MST:
            printf("Factory: Creating MST Strategy\n");
            return algorithm_get_strategy(3);
            
        case ALGO_MAX_CLIQUE:
            printf("Factory: Creating Max Clique Strategy\n");
            return algorithm_get_strategy(4);
            
        case ALGO_CLIQUE_COUNT:
            printf("Factory: Creating Clique Count Strategy\n");
            return algorithm_get_strategy(5);
            
        default:
            printf("Factory: Error - Unknown algorithm type %d\n", algo_type);
            return NULL;
    }
}

/**
 * Check if algorithm type is supported by factory.
 */
int algorithm_factory_is_supported(AlgorithmType algo_type) {
    return (algo_type >= ALGO_EULER && algo_type <= ALGO_CLIQUE_COUNT);
}

/**
 * Factory method to execute algorithm using both patterns together.
 */
char* algorithm_factory_execute(const Graph* g, int algorithm_id) {
    printf("Factory: Received request for algorithm ID %d\n", algorithm_id);
    
    // Step 1: Factory converts ID to type
    AlgorithmType algo_type = algorithm_factory_get_type(algorithm_id);
    if (algo_type == -1) {
        printf("Factory: Error - Invalid algorithm ID %d\n", algorithm_id);
        char* error_result = (char*)malloc(64);
        if (error_result) {
            snprintf(error_result, 64, "Factory Error: Invalid algorithm ID %d", algorithm_id);
        }
        return error_result;
    }
    
    printf("Factory: Converted ID %d to type %d\n", algorithm_id, algo_type);
    
    // Step 2: Factory checks if algorithm is supported
    if (!algorithm_factory_is_supported(algo_type)) {
        printf("Factory: Error - Algorithm type %d not supported\n", algo_type);
        char* error_result = (char*)malloc(64);
        if (error_result) {
            snprintf(error_result, 64, "Factory Error: Algorithm not supported");
        }
        return error_result;
    }
    
    // Step 3: Factory creates Strategy object
    AlgorithmStrategy* strategy = algorithm_factory_create_strategy(algo_type);
    if (!strategy) {
        printf("Factory: Error - Failed to create strategy\n");
        char* error_result = (char*)malloc(64);
        if (error_result) {
            snprintf(error_result, 64, "Factory Error: Strategy creation failed");
        }
        return error_result;
    }
    
    printf("Factory: Successfully created strategy '%s'\n", strategy->name);
    printf("Factory: Strategy description: '%s'\n", strategy->description);
    
    // Step 4: Factory delegates to Strategy pattern for execution
    printf("Strategy: Executing algorithm '%s'\n", strategy->name);
    
    // Use Strategy pattern to execute
    AlgorithmContext context;
    algorithm_context_init(&context, g);
    algorithm_context_set_strategy(&context, strategy);
    char* result = algorithm_context_execute(&context);
    
    if (result) {
        printf("Strategy: Execution successful\n");
        printf("Factory: Received result from strategy\n");
    } else {
        printf("Strategy: Execution failed\n");
        printf("Factory: Strategy returned null result\n");
    }
    
    return result;
}

/**
 * Print available algorithms that the factory can create.
 */
void algorithm_factory_print_available(void) {
    printf("Algorithm Factory - Available Products:\n");
    printf("ID  Type         Strategy Description\n");
    printf("--  -----------  --------------------\n");
    printf("1   EULER        Find Euler Circuit\n");
    printf("2   MAX_FLOW     Maximum Flow (Weighted)\n");
    printf("3   MST          Min Spanning Tree (Weighted)\n");
    printf("4   MAX_CLIQUE   Maximum Clique\n");
    printf("5   CLIQUE_COUNT Count All Cliques\n");
}

// Legacy functions for backward compatibility
char* algorithm_execute(const Graph* g, AlgorithmType algo_type) {
    return algorithm_factory_execute(g, (int)algo_type);
}

char* algorithm_execute_with_params(const Graph* g, AlgorithmType algo_type, int param1, int param2) {
    return algorithm_factory_execute(g, (int)algo_type);
}
