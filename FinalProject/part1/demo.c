#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

/* Helper: add edge and print result */
static void add_edge_and_report(Graph* g, int u, int v) {
    int rc = graph_add_edge(g, u, v);
    printf("add_edge(%d, %d) -> %d", u, v, rc);
    switch (rc) {
        case 0:  printf("  [OK]\n"); break;
        case -1: printf("  [ERROR: out of bounds]\n"); break;
        case -2: printf("  [ERROR: out of memory]\n"); break;
        case -3: printf("  [SKIPPED: duplicate edge / second self-loop]\n"); break;
        default: printf("  [UNKNOWN]\n"); break;
    }
}

int main(void) {
    /* Create graph with 5 vertices: 0..4 */
    Graph* g = graph_create(5);
    if (!g) {
        fprintf(stderr, "Failed to create graph\n");
        return 1;
    }

    /* Add some edges */
    add_edge_and_report(g, 0, 1);
    add_edge_and_report(g, 0, 2);
    add_edge_and_report(g, 1, 2);
    add_edge_and_report(g, 4, 2);

    /* One self-loop on 3 is allowed */
    add_edge_and_report(g, 3, 3);

    /* Duplicate edge (0,1) should be blocked (-3) */
    add_edge_and_report(g, 0, 1);

    /* Out-of-bounds should return -1 */
    add_edge_and_report(g, 4, 5);

    /* Print the final adjacency lists */
    printf("\nAdjacency lists:\n");
    graph_print(g);

    graph_destroy(g);
    return 0;
}
