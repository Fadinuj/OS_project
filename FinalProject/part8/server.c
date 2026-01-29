#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "../part7/graph.h"
#include "../part7/factory.h"
#define THREAD_POOL_SIZE 4
#define BUFFER_SIZE 4096

static int listener_fd;
static pthread_mutex_t leader_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t leader_cond = PTHREAD_COND_INITIALIZER;
static int current_leader = 0;
static volatile int shutdown_flag = 0;
static int total_requests = 0;

/* Send response to client */
static void send_response(int client_fd, const char* result) {
    if (!result) {
        int response[2] = {0, 0};
        send(client_fd, response, sizeof(response), 0);
        return;
    }
    
    int len = strlen(result);
    int total_size = 2 * sizeof(int) + len + 1;
    char* buffer = malloc(total_size);
    
    int* header = (int*)buffer;
    header[0] = 1;    // success
    header[1] = len;  // length
    strcpy(buffer + 2 * sizeof(int), result);
    
    send(client_fd, buffer, total_size, 0);
    free(buffer);
}

/* Process weighted algorithm request */
static void process_weighted_request(int client_fd, int* data, int size) {
    if (size < 3) {
        send_response(client_fd, NULL);
        return;
    }
    
    int algorithm_id = data[0];
    int n = data[1];
    int num_edges = data[2];
    
    printf("  Processing weighted algorithm %d: %d vertices, %d edges\n", 
           algorithm_id, n, num_edges);
    
    if (n <= 0 || n > 20 || num_edges < 0 || size < 3 + num_edges * 3) {
        send_response(client_fd, NULL);
        return;
    }
    
    // Create graph (separate for each client - no shared state!)
    Graph* g = graph_create(n);
    if (!g) {
        send_response(client_fd, NULL);
        return;
    }
    
    // Add weighted edges
    for (int i = 0; i < num_edges; i++) {
        int src = data[3 + i * 3];
        int dest = data[3 + i * 3 + 1];
        int weight = data[3 + i * 3 + 2];
        
        if (src >= 0 && src < n && dest >= 0 && dest < n && weight > 0) {
            graph_add_edge(g, src, dest);
            // Update weights manually
            for (EdgeNode* edge = g->adj[src].head; edge; edge = edge->next) {
                if (edge->to == dest) edge->weight = weight;
            }
            if (src != dest) {
                for (EdgeNode* edge = g->adj[dest].head; edge; edge = edge->next) {
                    if (edge->to == src) edge->weight = weight;
                }
            }
        }
    }
    
    // Execute using Factory Pattern from part 7
    char* result = algorithm_factory_execute(g, algorithm_id);
    send_response(client_fd, result);
    
    if (result) free(result);
    graph_destroy(g);
}

/* Process unweighted algorithm request */
static void process_unweighted_request(int client_fd, int* data, int size) {
    if (size < 2) {
        send_response(client_fd, NULL);
        return;
    }
    
    int algorithm_id = data[0];
    int n = data[1];
    
    printf("  Processing unweighted algorithm %d: %d vertices\n", algorithm_id, n);
    
    if (n <= 0 || n > 20 || size < 2 + n * n) {
        send_response(client_fd, NULL);
        return;
    }
    
    // Create graph (separate for each client)
    Graph* g = graph_create(n);
    if (!g) {
        send_response(client_fd, NULL);
        return;
    }
    
    // Build from adjacency matrix
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (data[2 + i * n + j] == 1) {
                graph_add_edge(g, i, j);
            }
        }
    }
    
    // Execute using Factory Pattern from part 7
    char* result = algorithm_factory_execute(g, algorithm_id);
    send_response(client_fd, result);
    
    if (result) free(result);
    graph_destroy(g);
}

/* Process single client request */
static void process_client(int client_fd) {
    int buffer[BUFFER_SIZE / sizeof(int)];
    int bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
    
    if (bytes <= 0) {
        close(client_fd);
        return;
    }
    
    int size = bytes / sizeof(int);
    if (size < 1) {
        send_response(client_fd, NULL);
        close(client_fd);
        return;
    }
    
    int algorithm_id = buffer[0];
    if (algorithm_id < 1 || algorithm_id > 5) {
        send_response(client_fd, NULL);
        close(client_fd);
        return;
    }
    
    // Route to appropriate handler
    if (algorithm_id == 2 || algorithm_id == 3) {
        process_weighted_request(client_fd, buffer, size);
    } else {
        process_unweighted_request(client_fd, buffer, size);
    }
    
    close(client_fd);
    total_requests++;
}

/* Leader-Follower worker thread */
static void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    free(arg);
    
    printf("[LF] Thread %d started\n", thread_id);
    
    while (!shutdown_flag) {
        pthread_mutex_lock(&leader_mutex);
        
        // Wait to become leader
        while (current_leader != thread_id && !shutdown_flag) {
            pthread_cond_wait(&leader_cond, &leader_mutex);
        }
        
        if (shutdown_flag) {
            pthread_mutex_unlock(&leader_mutex);
            break;
        }
        
        printf("Thread %d is Leader - accepting connections\n", thread_id);
        pthread_mutex_unlock(&leader_mutex);
        
        // Accept connection (as leader)
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(listener_fd, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client_fd >= 0) {
            printf("[LF] Leader %d accepted client %s:%d\n", 
                   thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            // Promote next leader immediately
            pthread_mutex_lock(&leader_mutex);
            current_leader = (current_leader + 1) % THREAD_POOL_SIZE;
            printf("[LF] Promoted thread %d to Leader\n", current_leader);
            pthread_cond_broadcast(&leader_cond);
            pthread_mutex_unlock(&leader_mutex);
            
            // Process client (now as worker, not leader)
            printf("[LF] Thread %d processing as Worker\n", thread_id);
            process_client(client_fd);
            printf("[LF] Thread %d finished processing\n", thread_id);
        }
    }
    
    printf("[LF] Thread %d exiting\n", thread_id);
    return NULL;
}

/* Signal handler */
static void signal_handler(int sig) {
    printf("\nShutting down server...\n");
    shutdown_flag = 1;
    pthread_cond_broadcast(&leader_cond);
}

/* Main function */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    signal(SIGINT, signal_handler);
    
    printf("=== Simple Leader-Follower Server ===\n");
    printf("Port: %d, Threads: %d\n", port, THREAD_POOL_SIZE);
    
    // Create server socket
    listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(listener_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    
    if (listen(listener_fd, 10) < 0) {
        perror("listen");
        return 1;
    }
    
    printf("Server listening...\n");
    
    // Create thread pool
    pthread_t threads[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        int* thread_id = malloc(sizeof(int));
        *thread_id = i;
        pthread_create(&threads[i], NULL, worker_thread, thread_id);
    }
    
    printf("[LF] Thread 0 is initial Leader\n");
    printf("Press Ctrl+C to shutdown\n\n");
    

    
    // Cleanup
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(threads[i], NULL);
    }
    
    close(listener_fd);
    printf("Server stopped. Total requests: %d\n", total_requests);
    return 0;
}