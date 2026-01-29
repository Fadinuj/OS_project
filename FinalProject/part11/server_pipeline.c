#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

// Include part 7 headers
#include "../part7/graph.h"
#include "../part7/mst.h"
#include "../part7/maxflow.h"
#include "../part7/maxclique.h"
#include "../part7/cliquecount.h"

#define PORT 3490
#define BACKLOG 10
#define MAX_QUEUE 32
#define MAX_EDGES 1000

// === Job Structure ===
typedef struct {
    int job_id;
    Graph *graph;
    int client_sock;
    time_t start_time;
    
    // Results from each stage
    char mst_result[256];
    char maxflow_result[256];
    char maxclique_result[256];
    char cliquecount_result[256];
    
    char final_response[2048];
} Job;

// === Thread-Safe Blocking Queue ===
typedef struct {
    Job* queue[MAX_QUEUE];
    int head, tail, count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty, not_full;
    char name[32];
} BlockingQueue;

// === Pipeline Stages (Queues) ===
BlockingQueue stage1_queue; // MST
BlockingQueue stage2_queue; // MaxFlow
BlockingQueue stage3_queue; // MaxClique
BlockingQueue stage4_queue; // CliqueCount

// === Global State ===
volatile int shutdown_flag = 0;
static int next_job_id = 1;
pthread_mutex_t job_id_mutex = PTHREAD_MUTEX_INITIALIZER;

// === Queue Management Functions ===
void queue_init(BlockingQueue *q, const char* name) {
    q->head = q->tail = q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
    strncpy(q->name, name, sizeof(q->name) - 1);
    printf("[Pipeline] Initialized queue: %s\n", q->name);
}

void queue_push(BlockingQueue *q, Job *job) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->count == MAX_QUEUE && !shutdown_flag) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    
    if (shutdown_flag) {
        pthread_mutex_unlock(&q->mutex);
        return;
    }
    
    q->queue[q->tail] = job;
    q->tail = (q->tail + 1) % MAX_QUEUE;
    q->count++;
    
    printf("[Pipeline] Job %d added to %s (queue size: %d)\n", 
           job->job_id, q->name, q->count);
    
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

Job* queue_pop(BlockingQueue *q) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->count == 0 && !shutdown_flag) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }
    
    if (shutdown_flag) {
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }
    
    Job* job = q->queue[q->head];
    q->head = (q->head + 1) % MAX_QUEUE;
    q->count--;
    
    printf("[Pipeline] Job %d removed from %s (queue size: %d)\n", 
           job->job_id, q->name, q->count);
    
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    return job;
}

// === Stage 1: MST Computation ===
void* stage1_mst_worker(void *arg) {
    printf("[Stage 1] MST worker started\n");
    
    while (!shutdown_flag) {
        Job* job = queue_pop(&stage1_queue);
        if (!job) continue;
        
        printf("[Stage 1] Processing Job %d - MST Algorithm\n", job->job_id);
        
        MST_Result mst_result;
        int success = graph_mst_prim(job->graph, &mst_result);
        
        if (success && mst_result.is_connected) {
            snprintf(job->mst_result, sizeof(job->mst_result),
                     "MST: Weight=%d, Edges=%d", 
                     mst_result.total_weight, mst_result.num_edges);
            mst_result_free(&mst_result);
        } else {
            snprintf(job->mst_result, sizeof(job->mst_result),
                     "MST: Graph not connected or calculation failed");
        }
        
        printf("[Stage 1] Job %d MST completed: %s\n", job->job_id, job->mst_result);
        
        // Pass to next stage
        queue_push(&stage2_queue, job);
    }
    
    printf("[Stage 1] MST worker shutting down\n");
    return NULL;
}

// === Stage 2: MaxFlow Computation ===
void* stage2_maxflow_worker(void *arg) {
    printf("[Stage 2] MaxFlow worker started\n");
    
    while (!shutdown_flag) {
        Job* job = queue_pop(&stage2_queue);
        if (!job) continue;
        
        printf("[Stage 2] Processing Job %d - MaxFlow Algorithm\n", job->job_id);
        
        int flow_value;
        int success = graph_max_flow_default(job->graph, &flow_value);
        
        if (success) {
            snprintf(job->maxflow_result, sizeof(job->maxflow_result),
                     "MaxFlow: Value=%d (source=0, sink=%d)", 
                     flow_value, job->graph->n - 1);
        } else {
            snprintf(job->maxflow_result, sizeof(job->maxflow_result),
                     "MaxFlow: Calculation failed");
        }
        
        printf("[Stage 2] Job %d MaxFlow completed: %s\n", job->job_id, job->maxflow_result);
        
        // Pass to next stage
        queue_push(&stage3_queue, job);
    }
    
    printf("[Stage 2] MaxFlow worker shutting down\n");
    return NULL;
}

// === Stage 3: MaxClique Computation ===
void* stage3_maxclique_worker(void *arg) {
    printf("[Stage 3] MaxClique worker started\n");
    
    while (!shutdown_flag) {
        Job* job = queue_pop(&stage3_queue);
        if (!job) continue;
        
        printf("[Stage 3] Processing Job %d - MaxClique Algorithm\n", job->job_id);
        
        int clique_size;
        int success = graph_max_clique_size(job->graph, &clique_size);
        
        if (success) {
            snprintf(job->maxclique_result, sizeof(job->maxclique_result),
                     "MaxClique: Size=%d", clique_size);
        } else {
            snprintf(job->maxclique_result, sizeof(job->maxclique_result),
                     "MaxClique: Calculation failed");
        }
        
        printf("[Stage 3] Job %d MaxClique completed: %s\n", job->job_id, job->maxclique_result);
        
        // Pass to next stage
        queue_push(&stage4_queue, job);
    }
    
    printf("[Stage 3] MaxClique worker shutting down\n");
    return NULL;
}

// === Stage 4: CliqueCount Computation & Response ===
void* stage4_cliquecount_worker(void *arg) {
    printf("[Stage 4] CliqueCount worker started\n");
    
    while (!shutdown_flag) {
        Job* job = queue_pop(&stage4_queue);
        if (!job) continue;
        
        printf("[Stage 4] Processing Job %d - CliqueCount Algorithm\n", job->job_id);
        
        int total_cliques;
        int success = graph_total_clique_count(job->graph, &total_cliques);
        
        if (success) {
            snprintf(job->cliquecount_result, sizeof(job->cliquecount_result),
                     "CliqueCount: Total=%d", total_cliques);
        } else {
            snprintf(job->cliquecount_result, sizeof(job->cliquecount_result),
                     "CliqueCount: Calculation failed");
        }
        
        printf("[Stage 4] Job %d CliqueCount completed: %s\n", job->job_id, job->cliquecount_result);
        
        // Build final response
        time_t end_time = time(NULL);
        double processing_time = difftime(end_time, job->start_time);
        
        snprintf(job->final_response, sizeof(job->final_response),
                 "=== PIPELINE PROCESSING RESULTS ===\n"
                 "Job ID: %d\n"
                 "Graph: %d vertices\n"
                 "Processing Time: %.2f seconds\n"
                 "\n=== ALGORITHM RESULTS ===\n"
                 "%s\n"
                 "%s\n"
                 "%s\n"
                 "%s\n"
                 "=====================================\n",
                 job->job_id, job->graph->n, processing_time,
                 job->mst_result, job->maxflow_result, 
                 job->maxclique_result, job->cliquecount_result);
        
        // Send response to client
        printf("[Stage 4] Sending response to client for Job %d\n", job->job_id);
        send(job->client_sock, job->final_response, strlen(job->final_response), 0);
        close(job->client_sock);
        
        // Cleanup
        printf("[Stage 4] Job %d completed and cleaned up\n", job->job_id);
        graph_destroy(job->graph);
        free(job);

    }
    
    printf("[Stage 4] CliqueCount worker shutting down\n");
    return NULL;
}

// === Client Request Handler ===
void* handle_client_request(void *arg) {
    int client_sock = *(int*)arg;
    free(arg);
    
    printf("[Client] New client connection handler started\n");
    
    // Receive header: [seed][max_weight][vertices]
    int header[3];
    int bytes_received = recv(client_sock, header, sizeof(header), 0);
    if (bytes_received != sizeof(header)) {
        printf("[Client] Failed to receive complete header\n");
        close(client_sock);
        return NULL;
    }
    
    int seed = header[0];
    int max_weight = header[1];
    int vertices = header[2];
    
    printf("[Client] Header received - Seed: %d, MaxWeight: %d, Vertices: %d\n", 
           seed, max_weight, vertices);
    
    if (vertices <= 0 || vertices > 50) {
        printf("[Client] Invalid vertex count: %d\n", vertices);
        close(client_sock);
        return NULL;
    }
    
    // Create graph
    Graph* graph = graph_create(vertices);
    if (!graph) {
        printf("[Client] Failed to create graph\n");
        close(client_sock);
        return NULL;
    }
    
    // Receive edges: variable number of [u][v][w] triplets
    int edges_buffer[MAX_EDGES][3];
    bytes_received = recv(client_sock, edges_buffer, sizeof(edges_buffer), 0);
    
    if (bytes_received > 0) {
        int num_edges = bytes_received / (3 * sizeof(int));
        printf("[Client] Received %d edges\n", num_edges);
        
        // Add edges to graph
        for (int i = 0; i < num_edges; i++) {
            int u = edges_buffer[i][0];
            int v = edges_buffer[i][1];
            int weight = edges_buffer[i][2];
            
            if (u >= 0 && u < vertices && v >= 0 && v < vertices && weight > 0) {
                graph_add_edge(graph, u, v);
                // Update weights manually (assuming your graph structure supports it)
                for (EdgeNode* edge = graph->adj[u].head; edge; edge = edge->next) {
                    if (edge->to == v) edge->weight = weight;
                }
                if (u != v) {
                    for (EdgeNode* edge = graph->adj[v].head; edge; edge = edge->next) {
                        if (edge->to == u) edge->weight = weight;
                    }
                }
            }
        }
    }
    
    // Create job
    Job* job = malloc(sizeof(Job));
    memset(job, 0, sizeof(Job));
    
    pthread_mutex_lock(&job_id_mutex);
    job->job_id = next_job_id++;
    pthread_mutex_unlock(&job_id_mutex);
    
    job->graph = graph;
    job->client_sock = client_sock;
    job->start_time = time(NULL);
    
    printf("[Client] Created Job %d, entering pipeline\n", job->job_id);
    
    // Enter pipeline at stage 1
    queue_push(&stage1_queue, job);
    
    return NULL;
}

// === Signal Handler ===
void signal_handler(int sig) {
    printf("\n[Main] Received signal %d, shutting down pipeline...\n", sig);
    shutdown_flag = 1;
    
    // Wake up all workers
    pthread_cond_broadcast(&stage1_queue.not_empty);
    pthread_cond_broadcast(&stage2_queue.not_empty);
    pthread_cond_broadcast(&stage3_queue.not_empty);
    pthread_cond_broadcast(&stage4_queue.not_empty);
}

// === Main Server ===
int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("=== Pipeline Pattern Graph Algorithm Server ===\n");
    printf("Using 4-stage pipeline: MST → MaxFlow → MaxClique → CliqueCount\n");
    printf("Listening on port %d\n", PORT);
    
    // Initialize pipeline queues
    queue_init(&stage1_queue, "MST_Queue");
    queue_init(&stage2_queue, "MaxFlow_Queue");
    queue_init(&stage3_queue, "MaxClique_Queue");
    queue_init(&stage4_queue, "CliqueCount_Queue");
    
    // Create pipeline worker threads
    pthread_t stage1_thread, stage2_thread, stage3_thread, stage4_thread;
    
    pthread_create(&stage1_thread, NULL, stage1_mst_worker, NULL);
    pthread_create(&stage2_thread, NULL, stage2_maxflow_worker, NULL);
    pthread_create(&stage3_thread, NULL, stage3_maxclique_worker, NULL);
    pthread_create(&stage4_thread, NULL, stage4_cliquecount_worker, NULL);
    
    printf("[Pipeline] All 4 stage workers started\n");
    
    // Create server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }
    
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }
    
    printf("[Main] Server ready - Pipeline pattern active!\n\n");
    
    // Accept client connections
    while (!shutdown_flag) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        int client_sock = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            if (!shutdown_flag) perror("accept");
            continue;
        }
        
        printf("[Main] New client connected: %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        // Create thread to handle client
        int* client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_sock;
        
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client_request, client_sock_ptr);
        pthread_detach(client_thread);
    }
    
    // Cleanup
    printf("[Main] Waiting for pipeline workers to finish...\n");
    pthread_join(stage1_thread, NULL);
    pthread_join(stage2_thread, NULL);
    pthread_join(stage3_thread, NULL);
    pthread_join(stage4_thread, NULL);
    
    close(server_fd);
    printf("[Main] Pipeline server shutdown complete\n");
    
    return 0;
}