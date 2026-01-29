
CC      = gcc 
CFLAGS  = -O2 -Wall -std=c11 -D_DEFAULT_SOURCE -I../part7
LDLIBS  = -pthread

ALGO_DIR = ../part7
ALGO_SRCS = \
  $(ALGO_DIR)/algorithm_strategy.c \
  $(ALGO_DIR)/factory.c \
  $(ALGO_DIR)/maxflow.c \
  $(ALGO_DIR)/mst.c \
  $(ALGO_DIR)/maxclique.c \
  $(ALGO_DIR)/cliquecount.c \
  $(ALGO_DIR)/graph.c

all: server client

server: server.c $(ALGO_SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

client: client.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f server client
