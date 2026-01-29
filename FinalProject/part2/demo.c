#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

/* עוזר קטן להדפסת מעגל האוילר */
static void print_euler_cycle(const char* title, const Graph* g){
    printf("===== %s =====\n", title);
    graph_print(g);

    if (!graph_has_euler_circuit(g)) {
        printf("No Euler C I R C U I T\n\n");
        return;
    }

    int* cycle = NULL; int len = 0;
    if (graph_find_euler_circuit(g, &cycle, &len)) {
        printf("Euler C I R C U I T found. Length (vertices): %d\n", len);
        printf("Cycle: ");
        for (int i = 0; i < len; ++i) {
            printf("%d%s", cycle[i], (i+1<len) ? " -> " : "\n");
        }
        printf("\n");
        free(cycle);
    } else {
        printf("No Euler C I R C U I T (unexpected: failed to extract)\n\n");
    }
}

int main(void){
    /* ----- דוגמה 1: יש מעגל אוילר – ריבוע 0-1-2-3-0 ----- */
    Graph* g1 = graph_create(4);
    graph_add_edge(g1,0,1);
    graph_add_edge(g1,1,2);
    graph_add_edge(g1,2,3);
    graph_add_edge(g1,3,0);
    print_euler_cycle("Graph 1 (square 0-1-2-3-0)", g1);
    graph_destroy(g1);

    /* ----- דוגמה 2: אין מעגל אוילר – שרשרת 0-1-2-3 (שני קודקודים אי-זוגיים) ----- */
    Graph* g2 = graph_create(4);
    graph_add_edge(g2,0,1);
    graph_add_edge(g2,1,2);
    graph_add_edge(g2,2,3);
    print_euler_cycle("Graph 2 (path 0-1-2-3)", g2);
    graph_destroy(g2);

    /* ----- דוגמה 3: יש מעגל אוילר – משולש 0-1-2-0 ----- */
    Graph* g3 = graph_create(3);
    graph_add_edge(g3,0,1);
    graph_add_edge(g3,1,2);
    graph_add_edge(g3,2,0);
    print_euler_cycle("Graph 3 (triangle 0-1-2-0)", g3);
    graph_destroy(g3);

    return 0;
}
