#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "../part2/graph.h"

#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10
/* Send response back to client */
static int send_euler_response(int client_socket, const Graph* g) {
    int response_buffer[BUFFER_SIZE / sizeof(int)];
    int response_size = 0;
    
    if (!graph_has_euler_circuit(g)) {
        /* No Euler circuit exists */
        response_buffer[0] = 0;  /* status: failure */
        response_buffer[1] = 0;  /* length: 0 */
        response_size = 2 * sizeof(int);
        
        printf("  → No Euler circuit exists\n");
    } else {
        /* Find Euler circuit */
        int* cycle = NULL;
        int len = 0;
        
        if (graph_find_euler_circuit(g, &cycle, &len)) {
            response_buffer[0] = 1;  /* status: success */
            response_buffer[1] = len; /* cycle length */
            
            /* Copy cycle to response buffer */
            for (int i = 0; i < len && (2 + len) * sizeof(int) < BUFFER_SIZE; i++) {
                response_buffer[2 + i] = cycle[i];
            }
            
            response_size = (2 + len) * sizeof(int);
            
            printf("  → Euler circuit found: ");
            for (int i = 0; i < len; i++) {
                printf(i == len-1 ? "%d\n" : "%d->", cycle[i]);
            }
            
            free(cycle);
        } else {
            /* Algorithm failed */
            response_buffer[0] = 0;  /* status: failure */
            response_buffer[1] = 0;  /* length: 0 */
            response_size = 2 * sizeof(int);
            
            printf("  → Algorithm failed to find circuit\n");
        }
    }
    
    /* Send response */
    int bytes_sent = send(client_socket, response_buffer, response_size, 0);
    if (bytes_sent != response_size) {
        printf("  → Warning: Could not send complete response\n");
        return -1;
    }
    
    return 0;
}

/* Process client request */
static int process_request(int client_socket, int* buffer, int bytes_received) {
    /* Validate minimum data size */
    if (bytes_received < (int)sizeof(int)) {
        printf("  → Error: Incomplete request (too small)\n");
        return -1;
    }
    
    int n = buffer[0];
    printf("  → Processing graph with %d vertices\n", n);
    
    /* Validate number of vertices */
    if (n <= 0 || n > 50) {  /* Reasonable limit */
        printf("  → Error: Invalid vertex count: %d\n", n);
        return -1;
    }
    
    /* Check if we received complete adjacency matrix */
    int expected_size = (1 + n * n) * sizeof(int);
    if (bytes_received < expected_size) {
        printf("  → Error: Incomplete matrix (expected %d bytes, got %d)\n", 
               expected_size, bytes_received);
        return -1;
    }
    
    /* Create graph */
    Graph* g = graph_create(n);
    if (!g) {
        printf("  → Error: Failed to create graph\n");
        return -1;
    }
    
    /* Build graph from adjacency matrix */
    int edges_added = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {  /* Only upper triangle + diagonal to avoid duplicates */
            int matrix_value = buffer[1 + i * n + j];
            if (matrix_value == 1) {
                int result = graph_add_edge(g, i, j);
                if (result == 0) {
                    edges_added++;
                } else if (result == -3) {
                    /* Duplicate edge - might happen if matrix isn't symmetric */
                    printf("  → Warning: Duplicate edge %d-%d ignored\n", i, j);
                }
            }
        }
    }
    
    printf("  → Graph built: %d edges added\n", edges_added);
    
    /* Debug: print graph structure */
    printf("  → Graph structure:\n");
    for (int i = 0; i < n && i < 10; i++) {  /* Limit output for large graphs */
        printf("    %d:", i);
        /* Simple degree count */
        int degree = 0;
        /* This is a bit hacky but avoids exposing internals */
        for (int j = 0; j < n; j++) {
            if (i == j && buffer[1 + i * n + j] == 1) degree += 2; /* self-loop */
            else if (i != j && buffer[1 + i * n + j] == 1) degree += 1;
        }
        printf(" degree=%d\n", degree);
    }
    
    /* Send response */
    send_euler_response(client_socket, g);
    
    /* Cleanup */
    graph_destroy(g);
    return 0;
}

/* Handle single client connection */
static void handle_client(int client_socket, struct sockaddr_in* client_addr) {
    printf("Client connected from %s:%d\n", 
           inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
    
    int buffer[BUFFER_SIZE / sizeof(int)];
    
    while (1) {
        /* Receive request */
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
        
        /* Process the request */
        if (process_request(client_socket, buffer, bytes_received) < 0) {
            printf("Failed to process request\n");
            /* Could send error response here */
        }
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
    
    printf("=== Euler Circuit Server ===\n");
    printf("Starting server on port %d...\n", port);
    
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
    printf("Protocol: [n][n×n matrix] → [status][length][cycle...]\n");
    printf("Ready to accept connections...\n\n");
    
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
        
        handle_client(client_socket, &client_addr);
    }
    
    close(server_fd);
    return 0;
}