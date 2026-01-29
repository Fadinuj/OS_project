#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "graph.h"
#include "factory.h"

#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10


/* Send response back to client */
static int send_algorithm_response(int client_socket, const char* result) {
    if (!result) {
        // Send failure response
        int response[2] = {0, 0}; // status=0, length=0
        send(client_socket, response, 2 * sizeof(int), 0);
        return 0;
    }
    
    int result_len = strlen(result);
    int total_size = 2 * sizeof(int) + result_len + 1; // status + length + string + null terminator
    
    char* response_buffer = (char*)malloc(total_size);
    if (!response_buffer) return 0;
    
    // Pack response: [status][length][string]
    int* header = (int*)response_buffer;
    header[0] = 1;          // status: success
    header[1] = result_len; // string length
    
    // Copy result string after header
    strcpy(response_buffer + 2 * sizeof(int), result);
    
    int bytes_sent = send(client_socket, response_buffer, total_size, 0);
    free(response_buffer);
    
    return (bytes_sent == total_size) ? 1 : 0;
}

/* Process MST request with weighted edges - FIXED VERSION */
static int process_mst_weighted_request(int client_socket, int* buffer, int bytes_received) {
    // Validate minimum data size: [algorithm_id][n][num_edges]
    if (bytes_received < 3 * (int)sizeof(int)) {
        printf("  → Error: MST request too small\n");
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    int algorithm_id = buffer[0];
    int n = buffer[1];
    int num_edges = buffer[2];
    
    printf("  → Max Flow/MST Algorithm: %d vertices, %d weighted edges\n", n, num_edges);
    
    // Validate data
    if (n <= 0 || n > 50) {
        printf("  → Error: Invalid vertex count: %d\n", n);
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    if (num_edges < 0 || num_edges > n * n) {
        printf("  → Error: Invalid edge count: %d\n", num_edges);
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    // Check if we received complete edge data
    int expected_size = (3 + num_edges * 3) * sizeof(int);
    if (bytes_received < expected_size) {
        printf("  → Error: Incomplete edge data (expected %d bytes, got %d)\n", 
               expected_size, bytes_received);
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    // Create graph
    Graph* g = graph_create(n);
    if (!g) {
        printf("  → Error: Failed to create graph\n");
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    // Add weighted edges from edge list
    // We'll create a weighted adjacency matrix approach since graph_add_weighted_edge might not be available
    int edges_added = 0;
    int edges_failed = 0;
    
    // Track edges we want to add for weighted processing
    typedef struct {
        int src, dest, weight;
    } WeightedEdge;
    
    WeightedEdge* weighted_edges = (WeightedEdge*)malloc(num_edges * sizeof(WeightedEdge));
    if (!weighted_edges) {
        printf("  → Error: Memory allocation failed\n");
        graph_destroy(g);
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    int valid_edges = 0;
    
    for (int i = 0; i < num_edges; i++) {
        int src = buffer[3 + i * 3];
        int dest = buffer[3 + i * 3 + 1];
        int weight = buffer[3 + i * 3 + 2];
        
        printf("    Processing edge: %d-%d (weight: %d)\n", src, dest, weight);
        
        // Validate edge
        if (src < 0 || src >= n || dest < 0 || dest >= n) {
            printf("    → Invalid edge vertices: %d-%d\n", src, dest);
            edges_failed++;
            continue;
        }
        
        if (weight <= 0) {
            printf("    → Invalid edge weight: %d\n", weight);
            edges_failed++;
            continue;
        }
        
        // Try to add edge using the basic function first
        int result = graph_add_edge(g, src, dest);
        if (result == 0) {
            // Store the weight information separately
            weighted_edges[valid_edges].src = src;
            weighted_edges[valid_edges].dest = dest;
            weighted_edges[valid_edges].weight = weight;
            valid_edges++;
            edges_added++;
            printf("    → Added edge %d-%d with weight %d\n", src, dest, weight);
        } else if (result == -3) {
            printf("    → Duplicate edge %d-%d ignored\n", src, dest);
            edges_failed++;
        } else {
            printf("    → Failed to add edge %d-%d (error: %d)\n", src, dest, result);
            edges_failed++;
        }
    }
    
    printf("  → Graph built: %d edges added, %d failed\n", edges_added, edges_failed);
    
    if (edges_added == 0) {
        printf("  → Error: No valid edges in graph\n");
        free(weighted_edges);
        graph_destroy(g);
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    // Now we need to manually update the weights in the graph structure
    // Since we can't use graph_add_weighted_edge, we'll modify the EdgeNode weights directly
    for (int i = 0; i < valid_edges; i++) {
        int src = weighted_edges[i].src;
        int dest = weighted_edges[i].dest;
        int weight = weighted_edges[i].weight;
        
        // Find and update the weight in both directions (since it's undirected)
        for (EdgeNode* edge = g->adj[src].head; edge; edge = edge->next) {
            if (edge->to == dest) {
                edge->weight = weight;
            }
        }
        
        if (src != dest) { // Don't double-update self-loops
            for (EdgeNode* edge = g->adj[dest].head; edge; edge = edge->next) {
                if (edge->to == src) {
                    edge->weight = weight;
                }
            }
        }
    }
    
    printf("  → Weights updated for all edges\n");
    
    // Execute algorithm using Factory + Strategy patterns
    printf("  → Using Factory Pattern to create Strategy and execute\n");
    char* result = algorithm_factory_execute(g, algorithm_id);
    
    if (result) {
        printf("  → MST result: %s\n", result);
        send_algorithm_response(client_socket, result);
        free(result);
    } else {
        printf("  → MST execution failed\n");
        send_algorithm_response(client_socket, NULL);
    }
    
    free(weighted_edges);
    graph_destroy(g);
    return 0;
}

/* Process standard unweighted request */
static int process_unweighted_request(int client_socket, int* buffer, int bytes_received) {
    // Validate minimum data size: [algorithm_id][n]
    if (bytes_received < 2 * (int)sizeof(int)) {
        printf("  → Error: Request too small\n");
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    int algorithm_id = buffer[0];
    int n = buffer[1];
    
    printf("  → Algorithm ID: %d, Vertices: %d (unweighted)\n", algorithm_id, n);
    
    // Validate data
    if (n <= 0 || n > 50) {
        printf("  → Error: Invalid vertex count: %d\n", n);
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    // Check if we received complete adjacency matrix
    int expected_size = (2 + n * n) * sizeof(int);
    if (bytes_received < expected_size) {
        printf("  → Error: Incomplete matrix (expected %d bytes, got %d)\n", 
               expected_size, bytes_received);
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    // Create graph from adjacency matrix
    Graph* g = graph_create(n);
    if (!g) {
        printf("  → Error: Failed to create graph\n");
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    // Build graph from adjacency matrix (starting at buffer[2])
    int edges_added = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) { // Upper triangle + diagonal
            int matrix_value = buffer[2 + i * n + j];
            if (matrix_value == 1) {
                int result = graph_add_edge(g, i, j);
                if (result == 0) {
                    edges_added++;
                }
            }
        }
    }
    
    printf("  → Graph built: %d edges added\n", edges_added);
    
    // Execute algorithm using Factory + Strategy patterns
    printf("  → Using Factory Pattern to create Strategy and execute\n");
    char* result = algorithm_factory_execute(g, algorithm_id);
    
    if (result) {
        printf("  → Algorithm result: %s\n", result);
        send_algorithm_response(client_socket, result);
        free(result);
    } else {
        printf("  → Algorithm execution failed\n");
        send_algorithm_response(client_socket, NULL);
    }
    
    graph_destroy(g);
    return 0;
}

/* Process client algorithm request - enhanced to handle both protocols */
static int process_algorithm_request(int client_socket, int* buffer, int bytes_received) {
    // Validate minimum data
    if (bytes_received < 1 * (int)sizeof(int)) {
        printf("  → Error: No algorithm ID received\n");
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    int algorithm_id = buffer[0];
    
    // Validate algorithm ID
    if (algorithm_id < 1 || algorithm_id > 5) {
        printf("  → Error: Invalid algorithm ID: %d\n", algorithm_id);
        send_algorithm_response(client_socket, NULL);
        return -1;
    }
    
    // Route to appropriate handler based on algorithm
    if (algorithm_id == 2 || algorithm_id == 3) { // Max Flow or MST - use weighted protocol
        return process_mst_weighted_request(client_socket, buffer, bytes_received);
    } else { // Others - use unweighted protocol
        return process_unweighted_request(client_socket, buffer, bytes_received);
    }
}

/* Handle single client connection */
static void handle_algorithm_client(int client_socket, struct sockaddr_in* client_addr) {
    printf("Client connected from %s:%d\n", 
           inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
    
    int buffer[BUFFER_SIZE / sizeof(int)];
    
    while (1) {
        // Receive request
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received == 0) {
            printf("Client disconnected gracefully\n");
            break;
        }
        
        if (bytes_received < 0) {
            if (errno == ECONNRESET) {
                printf("Client disconnected (connection reset)\n");
            } else {
                perror("recv failed");
            }
            break;
        }
        
        printf("Received %d bytes from client\n", bytes_received);
        
        // Process the algorithm request
        process_algorithm_request(client_socket, buffer, bytes_received);
    }
    
    close(client_socket);
    printf("Client connection closed\n\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port_number>\n", argv[0]);
        printf("Example: %s 8080\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        printf("Error: Invalid port number. Must be between 1-65535\n");
        return 1;
    }
    
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    printf("=== Enhanced Graph Algorithm Server (Factory + Strategy) ===\n");
    printf("Starting server on port %d...\n", port);
    
    // Print available algorithms using Factory
    algorithm_factory_print_available();
    
    /* Create socket */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    /* Set socket options */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    /* Configure address */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    /* Bind socket */
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    /* Listen for connections */
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d (max %d clients)\n", port, MAX_CLIENTS);
    printf("Ready to accept algorithm requests...\n\n");
    
    /* Main server loop */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        /* Accept connection */
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        /* Handle client */
        handle_algorithm_client(client_socket, &client_addr);
    }
    
    close(server_fd);
    return 0;
}