CC = gcc
CFLAGS = -Wall -std=c99 -I../part2
GRAPH = ../part2/graph.c

# Main targets
all: server client

# Algorithm server (Section 7) - using correct filenames
server: server.c algorithm_strategy.c factory.c maxflow.c mst.c maxclique.c cliquecount.c $(GRAPH)
	$(CC) $(CFLAGS) -o $@ $^

# Algorithm client - using correct filename
client: client.c
	$(CC) $(CFLAGS) -o $@ $^

# Clean
clean:
	rm -f server client

# Run targets
run_server: server
	./server 9090

run_client: client
	./client 9090

.PHONY: all clean run_server run_client