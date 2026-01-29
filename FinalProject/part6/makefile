CC = gcc
CFLAGS = -Wall -Wextra -std=c99
INCLUDES = -I../part2

# Source files
GRAPH_SRC = ../part2/graph.c
SERVER_SRC = euler_server.c
CLIENT_SRC =euler_client.c

# Executables
SERVER = euler_server
CLIENT = euler_client

# Default target
all: $(SERVER) $(CLIENT)

# Build server
$(SERVER): $(SERVER_SRC) $(GRAPH_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Build client  
$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Clean build files
clean:
	rm -f $(SERVER) $(CLIENT)