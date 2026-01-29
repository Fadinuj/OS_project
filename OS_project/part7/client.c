#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 8192  // הגדלה מ-4096 ל-8192

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port_number>\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        close(sock);
        return 1;
    }
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed. Make sure algorithm server is running on port %d\n", port);
        close(sock);
        return 1;
    }
    
    printf("=== Enhanced Algorithm Server Client ===\n");
    printf("Connected to server %s:%d\n\n", SERVER_IP, port);
    
    printf("Available algorithms:\n");
    printf("1. Euler Circuit (unweighted) - shows full path\n");
    printf("2. Max Flow (weighted) - shows flow value and source/sink\n");
    printf("3. MST Weight (weighted) - shows all edges with weights\n");
    printf("4. Max Clique (unweighted) - shows clique vertices\n");
    printf("5. Clique Count (unweighted) - shows detailed breakdown\n\n");
    
    while (1) {
        int algorithm_id, n;
        
        // Get algorithm choice
        printf("Enter algorithm ID (1-5, 0 to exit): ");
        scanf("%d", &algorithm_id);
        
        if (algorithm_id == 0) {
            printf("Exiting...\n");
            break;
        }
        
        if (algorithm_id < 1 || algorithm_id > 5) {
            printf("Invalid algorithm ID. Please enter 1-5.\n");
            continue;
        }
        
        // Get number of vertices
        printf("Enter number of vertices: ");
        scanf("%d", &n);
        
        if (n <= 0 || n > 10) {
            printf("Invalid number of vertices. Please enter 1-10.\n");
            continue;
        }
        
        // Different protocols based on algorithm
        if (algorithm_id == 2 || algorithm_id == 3) { // Max Flow or MST - weighted protocol
            printf("\n*** Max Flow/MST Algorithm - Weighted Graph Mode ***\n");
            
            // Weighted protocol: [algorithm_id][n][num_edges][edge_list]
            // Each edge: [src][dest][weight]
            
            int max_edges = n * (n - 1) / 2 + n; // Max possible edges including self-loops
            int* request = (int*)malloc((3 + max_edges * 3) * sizeof(int));
            if (!request) {
                printf("Memory allocation failed\n");
                continue;
            }
            
            request[0] = algorithm_id;
            request[1] = n;
            
            int edge_count = 0;
            int* edge_data = &request[3]; // Start of edge data
            
            printf("Enter weighted edges (format: src dest weight/capacity, enter -1 -1 -1 to finish):\n");
            while (1) {
                int src, dest, weight;
                printf("Edge: ");
                scanf("%d %d %d", &src, &dest, &weight);
                
                if (src == -1 && dest == -1 && weight == -1) {
                    break;
                }
                
                if (src < 0 || src >= n || dest < 0 || dest >= n) {
                    printf("Invalid edge. Vertices must be 0-%d\n", n-1);
                    continue;
                }
                
                if (weight <= 0) {
                    printf("Invalid weight/capacity. Must be positive.\n");
                    continue;
                }
                
                // Store edge data: [src][dest][weight]
                edge_data[edge_count * 3] = src;
                edge_data[edge_count * 3 + 1] = dest;
                edge_data[edge_count * 3 + 2] = weight;
                edge_count++;
                
                printf("Added edge %d-%d with weight/capacity %d\n", src, dest, weight);
            }
            
            request[2] = edge_count; // Number of edges
            
            if (edge_count == 0) {
                printf("No edges provided. Cannot compute algorithm.\n");
                free(request);
                continue;
            }
            
            // Send weighted request
            int request_size = (3 + edge_count * 3) * sizeof(int);
            printf("\nSending weighted request to server (%d edges)...\n", edge_count);
            
            int bytes_sent = send(sock, request, request_size, 0);
            if (bytes_sent != request_size) {
                printf("Failed to send complete request\n");
                free(request);
                continue;
            }
            
            free(request);
            
        } else { // Other algorithms - unweighted protocol
            printf("\n*** Unweighted Graph Mode ***\n");
            
            // Original protocol: [algorithm_id][n][matrix]
            int request_size = (2 + n * n) * sizeof(int);
            int* request = (int*)malloc(request_size);
            if (!request) {
                printf("Memory allocation failed\n");
                continue;
            }
            
            request[0] = algorithm_id;
            request[1] = n;
            
            // Initialize matrix to 0
            for (int i = 0; i < n * n; i++) {
                request[2 + i] = 0;
            }
            
            printf("Enter edges (format: src dest, enter -1 -1 to finish):\n");
            while (1) {
                int src, dest;
                printf("Edge: ");
                scanf("%d %d", &src, &dest);
                
                if (src == -1 && dest == -1) {
                    break;
                }
                
                if (src < 0 || src >= n || dest < 0 || dest >= n) {
                    printf("Invalid edge. Vertices must be 0-%d\n", n-1);
                    continue;
                }
                
                // Set edge in adjacency matrix (undirected)
                request[2 + src * n + dest] = 1;
                request[2 + dest * n + src] = 1;
                printf("Added edge %d-%d\n", src, dest);
            }
            
            // Send unweighted request
            printf("\nSending unweighted request to server...\n");
            int bytes_sent = send(sock, request, request_size, 0);
            if (bytes_sent != request_size) {
                printf("Failed to send complete request\n");
                free(request);
                continue;
            }
            
            free(request);
        }
        
        // Receive response (same for both protocols) - Enhanced version
        char response_buffer[BUFFER_SIZE];
        int bytes_received = recv(sock, response_buffer, BUFFER_SIZE - 1, 0);  // השאר מקום ל-null terminator
        
        if (bytes_received < 2 * (int)sizeof(int)) {
            printf("Invalid response from server\n");
            continue;
        }
        
        // Parse response: [status][length][result_string]
        int* header = (int*)response_buffer;
        int status = header[0];
        int result_length = header[1];
        
        // בדיקת תקינות אורך התוצאה
        if (result_length < 0 || result_length > BUFFER_SIZE - 2 * sizeof(int) - 1) {
            printf("Invalid result length from server: %d\n", result_length);
            continue;
        }
        
        if (status == 1 && result_length > 0) {
            char* result_string = response_buffer + 2 * sizeof(int);
            result_string[result_length] = '\0'; // Ensure null termination
            printf("✓ Detailed Result: %s\n", result_string);
        } else {
            printf("✗ Algorithm execution failed\n");
        }
        
        printf("\n");
    }
    
    close(sock);
    return 0;
}