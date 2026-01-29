#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port_number>\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address / Address not supported\n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }
    
    int n = 0;
    int src, dest;
    
    for (;;) {
        printf("Enter number of vertices (0 to exit): ");
        scanf("%d", &n);
        
        if (n == 0) {
            close(sock);
            printf("Connection closed.\n");
            break;
        }
        
        if (n < 0) {
            printf("n must be positive number\n");
            continue;
        }
        
        int arr[n * n + 1]; /* n*n for the neighbor matrix and extra place for the size */
        arr[0] = n;
        
        /* Initialize matrix to 0 */
        for (int i = 1; i < n * n + 1; i++) {
            arr[i] = 0;
        }
        
        printf("Now enter edges. Enter '0 0' to finish:\n");
        for (;;) {
            printf("Enter src dest: ");
            scanf("%d %d", &src, &dest);
            
            if (dest == src && dest == 0) {
                break;
            }
            
            if (dest < 0 || dest >= n || src < 0 || src >= n || src == dest) {
                printf("Illegal arguments: src,dest should be different numbers between 0 to %d\n", n-1);
                continue;
            }
            
            /* Set both directions for undirected graph */
            arr[dest * n + src + 1] = 1; /* dest*n + src to change from matrix to arr +1 because of size */
            arr[src * n + dest + 1] = 1; /* src*n + dest to change from matrix to arr +1 because of size */
        }
        
        /* Send request to server */
        printf("Sending graph to server...\n");
        size_t expected_bytes = (size_t)(n * n + 1) * sizeof(int);
        int bytes_sent = send(sock, &arr, expected_bytes, 0);
        if (bytes_sent != (int)expected_bytes) {
            printf("Error: Failed to send complete request\n");
            continue;
        }
        
        /* Receive response */
        printf("Waiting for server response...\n");
        int response[BUFFER_SIZE / sizeof(int)];
        int bytes_received = recv(sock, response, BUFFER_SIZE, 0);
        
        printf("Received %d bytes from server\n", bytes_received);
        
        if (bytes_received >= (int)(2 * sizeof(int))) {
            int status = response[0];
            int cycle_length = response[1];
            
            printf("Status: %d, Length: %d\n", status, cycle_length);
            
            if (status == 1 && cycle_length > 0) {
                printf("✓ Euler circuit found! Length: %d\n", cycle_length);
                printf("Circuit: ");
                for (int i = 0; i < cycle_length; i++) {
                    printf(i == cycle_length-1 ? "%d\n" : "%d->", response[2 + i]);
                }
            } else {
                printf("✗ No Euler circuit exists\n");
            }
        } else {
            printf("Error: Invalid response from server (got %d bytes)\n", bytes_received);
        }
        
        printf("\n");
    }
    
    return 0;
}