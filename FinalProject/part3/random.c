#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>

extern char *optarg;

/**
 * Print usage information and exit with error code 1.
 */
static void print_usage_and_exit(const char *program_name)
{
    fprintf(stderr, "Usage: %s -v numOfVertices(int) -e numOfEdges(int) -r randomSeed(int)\n", program_name);
    exit(1);
}

/**
 * Calculate maximum possible edges in a simple undirected graph with n vertices.
 * Formula: n*(n-1)/2 (no self-loops) or n*(n+1)/2 (with self-loops)
 * We'll use n*(n-1)/2 + n = n*(n+1)/2 to allow self-loops.
 */
static unsigned long calculate_max_edges(int n)
{
    return (unsigned long)n * (n + 1) / 2;
}

/**
 * Generate a random graph by repeatedly trying to add random edges.
 * Uses rejection sampling - keeps trying until we get the desired number of edges.
 */
static int generate_random_graph(Graph *g, int num_edges, int random_seed)
{
    if (!g)
        return -1;

    srand((unsigned int)random_seed);
    int edges_added = 0;
    int max_attempts = num_edges * 1000; // Prevent infinite loops
    int attempts = 0;

    while (edges_added < num_edges && attempts < max_attempts)
    {
        int u = rand() % g->n;
        int v = rand() % g->n;

        int result = graph_add_edge(g, u, v);
        if (result == 0)
        {
            edges_added++;
            printf("Added edge: %d -- %d (total: %d/%d)\n", u, v, edges_added, num_edges);
        }
        // If result == -3 (duplicate), just try again
        // If result == -1 or -2, that's an error
        else if (result == -1 || result == -2)
        {
            fprintf(stderr, "Error adding edge %d -- %d: %d\n", u, v, result);
            return -1;
        }

        attempts++;
    }

    if (edges_added < num_edges)
    {
        fprintf(stderr, "Warning: Could only add %d out of %d requested edges after %d attempts\n",
                edges_added, num_edges, attempts);
        return edges_added;
    }

    return edges_added;
}

int main(int argc, char *argv[])
{
    // Check if we have the right number of arguments
    if (argc != 7)
    { // program name + 6 arguments (-v val -e val -r val)
        print_usage_and_exit(argv[0]);
    }

    int opt;
    int num_vertices = -1;
    int num_edges = -1;
    int random_seed = -1;

    // Parse command line options
    while ((opt = getopt(argc, argv, "v:e:r:")) != -1)
    {
        switch (opt)
        {
        case 'v':
            num_vertices = atoi(optarg);
            break;
        case 'e':
            num_edges = atoi(optarg);
            break;
        case 'r':
            random_seed = atoi(optarg);
            break;
        default:
            print_usage_and_exit(argv[0]);
        }
    }

    // Validate that all required parameters were provided
    if (num_vertices == -1 || num_edges == -1 || random_seed == -1)
    {
        fprintf(stderr, "Error: All parameters (-v, -e, -r) must be provided\n");
        print_usage_and_exit(argv[0]);
    }

    // Validate parameter ranges
    if (num_vertices <= 0)
    {
        fprintf(stderr, "Error: Number of vertices must be positive (got %d)\n", num_vertices);
        exit(1);
    }

    if (num_edges < 0)
    {
        fprintf(stderr, "Error: Number of edges must be non-negative (got %d)\n", num_edges);
        exit(1);
    }

    // Check if the requested number of edges is feasible
    unsigned long max_edges = calculate_max_edges(num_vertices);
    if ((unsigned long)num_edges > max_edges)
    {
        fprintf(stderr, "Error: Too many edges requested\n");
        fprintf(stderr, "Requested: %d edges, Maximum possible: %lu edges\n", num_edges, max_edges);
        fprintf(stderr, "For %d vertices, maximum is %d*((%d+1)/2) = %lu\n",
                num_vertices, num_vertices, num_vertices, max_edges);
        exit(1);
    }

    printf("=== Random Graph Generation ===\n");
    printf("Vertices: %d\n", num_vertices);
    printf("Edges to generate: %d\n", num_edges);
    printf("Random seed: %d\n", random_seed);
    printf("Maximum possible edges: %lu\n\n", max_edges);

    // Create the graph
    Graph *g = graph_create(num_vertices);
    if (!g)
    {
        fprintf(stderr, "Error: Failed to create graph with %d vertices\n", num_vertices);
        exit(1);
    }

    // Generate random edges
    printf("Generating random edges...\n");
    int actual_edges = generate_random_graph(g, num_edges, random_seed);
    if (actual_edges < 0)
    {
        fprintf(stderr, "Error: Failed to generate random graph\n");
        graph_destroy(g);
        exit(1);
    }

    printf("\n=== Generated Graph ===\n");
    graph_print(g);

    // Check for Euler circuit
    printf("\n=== Euler Circuit Analysis ===\n");
    if (!graph_has_euler_circuit(g))
    {
        printf("No Euler circuit exists in this graph.\n");
        printf("(Either the graph is not connected, or some vertices have odd degree)\n");
    }
    else
    {
        printf("Euler circuit exists! Finding it...\n\n");

        int *cycle = NULL;
        int cycle_length = 0;

        if (graph_find_euler_circuit(g, &cycle, &cycle_length))
        {
            printf("=== Euler Circuit Found ===\n");
            printf("Circuit length (vertices): %d\n", cycle_length);
            printf("The circuit is:\n");

            for (int i = 0; i < cycle_length; i++)
            {
                if (i == cycle_length - 1)
                {
                    printf("%d\n", cycle[i]);
                }
                else
                {
                    printf("%d -> ", cycle[i]);
                    
                }
            }

            free(cycle);
        }
        else
        {
            printf("Error: Failed to extract Euler circuit (unexpected)\n");
        }
    }

    // Clean up
    graph_destroy(g);
    printf("\nDone!\n");

    return 0;
}