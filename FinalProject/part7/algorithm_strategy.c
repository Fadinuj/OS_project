#include "algorithm_strategy.h"
#include "maxflow.h"
#include "mst.h"
#include "maxclique.h"
#include "cliquecount.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Concrete Strategy Implementations
 */

static char* euler_strategy_execute(const Graph* g) {
    char* result = (char*)malloc(256);
    if (!result) return NULL;
    
    if (graph_has_euler_circuit(g)) {
        int* cycle = NULL;
        int len = 0;
        if (graph_find_euler_circuit(g, &cycle, &len)) {
            snprintf(result, 256, "Euler circuit found (length: %d)", len);
            free(cycle);
        } else {
            snprintf(result, 256, "Euler circuit exists but extraction failed");
        }
    } else {
        snprintf(result, 256, "No Euler circuit exists");
    }
    return result;
}

static char* maxflow_strategy_execute(const Graph* g) {
    char* result = (char*)malloc(256);
    if (!result) return NULL;
    
    int flow_value;
    if (graph_max_flow_default(g, &flow_value)) {
        snprintf(result, 256, "Max flow is: %d", flow_value);
    } else {
        snprintf(result, 256, "Max flow calculation failed");
    }
    return result;
}

static char* mst_strategy_execute(const Graph* g) {
    char* result = (char*)malloc(1024);  
    if (!result) return NULL;
    MST_Result mst_result;
    if (graph_mst_prim(g, &mst_result)) {
        if (mst_result.is_connected) {
            int offset = 0;
            offset += snprintf(result + offset, 1024 - offset, 
                             "MST weight: %d, Edges: ", mst_result.total_weight);
            
            for (int i = 0; i < mst_result.num_edges; i++) {
                if (i > 0) {
                    offset += snprintf(result + offset, 1024 - offset, ", ");
                }
                offset += snprintf(result + offset, 1024 - offset, 
                                 "%d-%d(%d)", 
                                 mst_result.edges[i].u, 
                                 mst_result.edges[i].v,
                                 mst_result.edges[i].weight);
                if (offset >= 1000) {
                    snprintf(result + 1000, 24, "...[truncated]");
                    break;
                }
            }
            
            mst_result_free(&mst_result);
        } else {
            snprintf(result, 1024, "MST calculation failed (graph not connected)");
        }
    } else {
        snprintf(result, 1024, "MST calculation failed");
    }
    return result;
}

static char* maxclique_strategy_execute(const Graph* g) {
    char* result = (char*)malloc(256);
    if (!result) return NULL;
    
    int clique_size;
    if (graph_max_clique_size(g, &clique_size)) {
        snprintf(result, 256, "Max clique size is: %d", clique_size);
    } else {
        snprintf(result, 256, "Max clique calculation failed");
    }
    return result;
}

static char* cliquecount_strategy_execute(const Graph* g) {
    char* result = (char*)malloc(256);
    if (!result) return NULL;
    
    int total_cliques;
    if (graph_total_clique_count(g, &total_cliques)) {
        snprintf(result, 256, "Total cliques count is: %d", total_cliques);
    } else {
        snprintf(result, 256, "Clique counting failed");
    }
    return result;
}

/**
 * Strategy Registry
 */
static AlgorithmStrategy strategies[] = {
    {euler_strategy_execute, "euler", "Find Euler Circuit", 1},
    {maxflow_strategy_execute, "maxflow", "Maximum Flow (Edmonds-Karp)", 2},
    {mst_strategy_execute, "mst", "Minimum Spanning Tree (Prim's)", 3},
    {maxclique_strategy_execute, "maxclique", "Maximum Clique", 4},
    {cliquecount_strategy_execute, "cliquecount", "Count All Cliques", 5}
};

static const int num_strategies = sizeof(strategies) / sizeof(strategies[0]);

/**
 * Algorithm Context Implementation
 */

void algorithm_context_init(AlgorithmContext* context, const Graph* graph) {
    if (context) {
        context->strategy = NULL;
        context->graph = graph;
    }
}

void algorithm_context_set_strategy(AlgorithmContext* context, AlgorithmStrategy* strategy) {
    if (context) {
        context->strategy = strategy;
    }
}

char* algorithm_context_execute(AlgorithmContext* context) {
    if (!context || !context->strategy || !context->graph) {
        return NULL;
    }
    
    return context->strategy->execute(context->graph);
}

AlgorithmStrategy* algorithm_get_strategy(int algorithm_id) {
    for (int i = 0; i < num_strategies; i++) {
        if (strategies[i].id == algorithm_id) {
            return &strategies[i];
        }
    }
    return NULL;
}

AlgorithmStrategy* algorithm_get_strategy_by_name(const char* algorithm_name) {
    if (!algorithm_name) return NULL;
    
    for (int i = 0; i < num_strategies; i++) {
        if (strcmp(strategies[i].name, algorithm_name) == 0) {
            return &strategies[i];
        }
    }
    return NULL;
}

AlgorithmStrategy** algorithm_get_all_strategies(int* count) {
    if (count) {
        *count = num_strategies;
    }
    
    AlgorithmStrategy** result = (AlgorithmStrategy**)malloc(num_strategies * sizeof(AlgorithmStrategy*));
    if (!result) return NULL;
    
    for (int i = 0; i < num_strategies; i++) {
        result[i] = &strategies[i];
    }
    
    return result;
}

void algorithm_print_strategies(void) {
    printf("Available Algorithm Strategies:\n");
    for (int i = 0; i < num_strategies; i++) {
        printf("  %d. %-12s - %s\n", 
               strategies[i].id, 
               strategies[i].name, 
               strategies[i].description);
    }
}

char* algorithm_execute_by_id(const Graph* graph, int algorithm_id) {
    AlgorithmContext context;
    algorithm_context_init(&context, graph);
    
    AlgorithmStrategy* strategy = algorithm_get_strategy(algorithm_id);
    if (!strategy) {
        char* result = (char*)malloc(64);
        if (result) {
            snprintf(result, 64, "Unknown algorithm ID: %d", algorithm_id);
        }
        return result;
    }
    
    algorithm_context_set_strategy(&context, strategy);
    return algorithm_context_execute(&context);
}