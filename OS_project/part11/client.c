/*
** client.c -- simple C client for pipeline server (binary protocol)
*/

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT "3490"      // port server is listening on
#define MAXDATASIZE 4096 // max bytes to receive

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int sockfd, rv;
    char s[INET6_ADDRSTRLEN];
    struct addrinfo hints, *servinfo, *p;

    int seed = time(NULL);
    int mode = -1; // 0=manual, 1=random
    int vertices = 0, edges = 0, max_weight = 10;

    int opt;
    while ((opt = getopt(argc, argv, "rmn:e:w:s:")) != -1) {
        switch (opt) {
            case 'r': mode = 1; break;
            case 'm': mode = 0; break;
            case 'n': vertices = atoi(optarg); break;
            case 'e': edges = atoi(optarg); break;
            case 'w': max_weight = atoi(optarg); break;
            case 's': seed = atoi(optarg); break;
            default:
                fprintf(stderr,
                    "Usage: %s [-r|-m] -n <vertices> -e <edges> [-w <max_weight>] [-s <seed>]\n",
                    argv[0]);
                return 1;
        }
    }

    if (mode == -1 || vertices <= 0 || (mode == 1 && edges <= 0)) {
        fprintf(stderr,
            "Usage: %s [-r|-m] -n <vertices> -e <edges> [-w <max_weight>] [-s <seed>]\n",
            argv[0]);
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connected to %s\n", s);

    freeaddrinfo(servinfo);

    // === Send header ===
    // header[0] = seed
    // header[1] = max_weight (אפשר לשנות בעתיד אם תרצה)
    // header[2] = מספר הצמתים
    int header[3];
    header[0] = seed;
    header[1] = max_weight;
    header[2] = vertices;

    if (send(sockfd, header, sizeof(header), 0) == -1) {
        perror("send header");
        close(sockfd);
        return 1;
    }

    // === Send edges ===
    int (*edges_arr)[3] = malloc(edges * sizeof(int[3]));
    if (!edges_arr) {
        perror("malloc");
        close(sockfd);
        return 1;
    }

    if (mode == 1) {
        srand(seed);
        int generated = 0;
        while (generated < edges) {
            int u = rand() % vertices;
            int v = rand() % vertices;
            if (u == v) continue; // no self-loops
            int w = (rand() % max_weight) + 1;
            edges_arr[generated][0] = u;
            edges_arr[generated][1] = v;
            edges_arr[generated][2] = w;
            generated++;
        }
    } else {
        for (int i = 0; i < edges; i++) {
            int u,v,w;
            printf("Enter edge %d (u v w): ", i+1);
            if (scanf("%d %d %d",&u,&v,&w)!=3) {
                fprintf(stderr,"Invalid input\n");
                free(edges_arr);
                close(sockfd);
                return 1;
            }
            edges_arr[i][0] = u;
            edges_arr[i][1] = v;
            edges_arr[i][2] = w;
        }
    }

    if (send(sockfd, edges_arr, edges * sizeof(int[3]), 0) == -1) {
        perror("send edges");
        free(edges_arr);
        close(sockfd);
        return 1;
    }

    free(edges_arr);

    // === Receive reply ===
    char result[MAXDATASIZE];
    int numbytes = recv(sockfd, result, sizeof(result)-1, 0);
    if (numbytes > 0) {
        result[numbytes] = '\0';
        printf("Result from server:\n%s\n", result);
    } else {
        printf("No reply from server.\n");
    }

    close(sockfd);
    return 0;
}
