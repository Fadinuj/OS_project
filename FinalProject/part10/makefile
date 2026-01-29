CC = gcc
CFLAGS = -g -O0 -Wall -pthread
TARGET = ../part9/server_pipeline

SRC = ../part9/server_pipeline.c ../part7/graph.c ../part7/mst.c ../part7/maxflow.c ../part7/maxclique.c ../part7/cliquecount.c

VALDIR = valgrind_analysis
MEMDIR = $(VALDIR)/memcheck
HELLDIR = $(VALDIR)/hellgrind
CALLDIR = $(VALDIR)/callgraph

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lpthread

# Run Valgrind Memcheck
memcheck: $(TARGET)
	mkdir -p $(MEMDIR)
	valgrind --leak-check=full --show-leak-kinds=all \
		--log-file=$(MEMDIR)/memcheck.log $(TARGET) 5555

# Run Helgrind
helgrind: $(TARGET)
	mkdir -p $(HELLDIR)
	valgrind --tool=helgrind \
		--log-file=$(HELLDIR)/helgrind.log $(TARGET) 5555

# Run Callgrind
callgrind: $(TARGET)
	mkdir -p $(CALLDIR)
	valgrind --tool=callgrind \
		--callgrind-out-file=$(CALLDIR)/callgrind.out $(TARGET) 5555

clean:
	rm -f $(TARGET)
	rm -rf $(VALDIR)/*
