#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SERVER_IP "127.0.0.1"

static int connect_to_server(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

static void send_request(int sock, int* request, int size) {
    send(sock, request, size * sizeof(int), 0);
    
    int header[2];
    recv(sock, header, sizeof(header), 0);
    
    if (header[0] == 1 && header[1] > 0) {
        char* result = malloc(header[1] + 1);
        recv(sock, result, header[1] + 1, 0);
        printf("%s\n", result);
        free(result);
    } else {
        printf("Server error\n");
    }
}

static void test_weighted(int port, int algorithm_id) {
    int n;
    printf("Vertices: ");
    if (scanf("%d", &n) != 1) return;
    
    int edges[50][3], count = 0;
    printf("Enter edges (src dest weight), -1 to finish:\n");
    
    while (count < 50) {
        int src, dest, weight;
        if (scanf("%d %d %d", &src, &dest, &weight) != 3) continue;
        if (src == -1) break;
        
        edges[count][0] = src;
        edges[count][1] = dest;
        edges[count][2] = weight;
        count++;
    }
    
    int size = 3 + count * 3;
    int* request = malloc(size * sizeof(int));
    request[0] = algorithm_id;
    request[1] = n;
    request[2] = count;
    
    for (int i = 0; i < count; i++) {
        request[3 + i*3] = edges[i][0];
        request[3 + i*3 + 1] = edges[i][1];
        request[3 + i*3 + 2] = edges[i][2];
    }
    
    int sock = connect_to_server(port);
    if (sock >= 0) {
        send_request(sock, request, size);
        close(sock);
    }
    free(request);
}

static void test_unweighted(int port, int algorithm_id) {
    int n;
    printf("Vertices: ");
    if (scanf("%d", &n) != 1) return;
    
    int* matrix = calloc(2 + n * n, sizeof(int));
    matrix[0] = algorithm_id;
    matrix[1] = n;
    
    printf("Enter edges (u v), -1 to finish:\n");
    while (1) {
        int u, v;
        if (scanf("%d %d", &u, &v) != 2) continue;
        if (u == -1) break;
        
        matrix[2 + u*n + v] = 1;
        matrix[2 + v*n + u] = 1;
    }
    
    int sock = connect_to_server(port);
    if (sock >= 0) {
        send_request(sock, matrix, 2 + n * n);
        close(sock);
    }
    free(matrix);
}

static void quick_test(int port, int algorithm_id) {
    int sock = connect_to_server(port);
    if (sock < 0) return;
    
    if (algorithm_id == 2 || algorithm_id == 3) {
        int request[] = {algorithm_id, 4, 3, 0,1,5, 1,2,3, 2,3,7};
        send_request(sock, request, 10);
    } else {
        int request[18] = {algorithm_id, 4};
        request[2 + 0*4 + 1] = request[2 + 1*4 + 0] = 1;
        request[2 + 1*4 + 2] = request[2 + 2*4 + 1] = 1;
        request[2 + 2*4 + 3] = request[2 + 3*4 + 2] = 1;
        request[2 + 3*4 + 0] = request[2 + 0*4 + 3] = 1;
        send_request(sock, request, 18);
    }
    close(sock);
}

static void test_concurrent(int port, int num) {
    for (int i = 0; i < num; i++) {
        if (fork() == 0) {
            quick_test(port, (i % 5) + 1);
            exit(0);
        }
    }
    for (int i = 0; i < num; i++) wait(NULL);
    printf("All %d clients finished\n", num);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    
    while (1) {
        printf("\n1.Euler 2.MaxFlow 3.MST 4.Clique 5.Count 6.Quick 7.Concurrent 0.Exit\n");
        printf("Choice: ");
        
        int choice;
        if (scanf("%d", &choice) != 1) continue;
        
        switch (choice) {
            case 0: return 0;
            case 1: case 4: case 5: test_unweighted(port, choice); break;
            case 2: case 3: test_weighted(port, choice); break;
            case 6: {
                printf("Algorithm: "); 
                int alg;
                if (scanf("%d", &alg) != 1) break;
                quick_test(port, alg); 
                break;
            }
            case 7: {
                printf("Clients: "); 
                int num;
                if (scanf("%d", &num) != 1) break;
                test_concurrent(port, num); 
                break;
            }
        }
    }
}