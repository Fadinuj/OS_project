CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pthread -D_POSIX_C_SOURCE=200112L
TARGET_CLIENT = client

# קבצי מקור
SERVER_SRC = server_pipeline.c \
             ../part7/graph.c \
             ../part7/maxflow.c \
             ../part7/mst.c \
             ../part7/maxclique.c \
             ../part7/cliquecount.c

CLIENT_SRC = client.c

# קבצי הרצה
SERVER_BIN = server
CLIENT_BIN = client

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_BIN)

run: $(TARGET_CLIENT)
	./$(TARGET_CLIENT) -r -n 5 -e 6

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) *.o
