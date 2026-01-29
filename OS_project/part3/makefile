CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -I../part2

random: random.o graph.o
	$(CC) $(CFLAGS) -o random random.o graph.o

random.o: random.c ../part2/graph.h
	$(CC) $(CFLAGS) -c random.c

graph.o: ../part2/graph.c ../part2/graph.h
	$(CC) $(CFLAGS) -c ../part2/graph.c

clean:
	rm -f *.o random

.PHONY: clean