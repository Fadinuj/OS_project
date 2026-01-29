CC = gcc
CFLAGS = -g -O0 -Wall -pthread --coverage
LDFLAGS = --coverage

SERVER = server_pipeline
CLIENT = client

SRCS_SERVER = server_pipeline.c ../part7/graph.c ../part7/mst.c ../part7/maxflow.c ../part7/maxclique.c ../part7/cliquecount.c
OBJS_SERVER = $(SRCS_SERVER:.c=.o)

SRCS_CLIENT = client.c
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)

COVERAGE_DIR = coverage_folder
HTML_DIR = coverage_html

all: $(SERVER) $(CLIENT)

$(SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# הרצה של server + clients (עם timeout)
run: all
	timeout 15 ./$(SERVER) 5555 & \
	sleep 1; \
	./$(CLIENT) -r -n 5 -e 6; \
	./$(CLIENT) -r -n 8 -e 12; \
	./$(CLIENT) -r -n 6 -e 7; \
	./$(CLIENT) -r -n 10 -e 15; \
	echo "Run finished, server stopped."

# הפקת דוחות כיסוי
coverage: run
	mkdir -p $(COVERAGE_DIR)
	gcov -o . server_pipeline.c > $(COVERAGE_DIR)/server_pipeline.txt
	gcov -o . client.c > $(COVERAGE_DIR)/client.txt
	gcov -o ../part7 ../part7/graph.c > $(COVERAGE_DIR)/graph.txt
	gcov -o ../part7 ../part7/mst.c > $(COVERAGE_DIR)/mst.txt
	gcov -o ../part7 ../part7/maxflow.c > $(COVERAGE_DIR)/maxflow.txt
	gcov -o ../part7 ../part7/maxclique.c > $(COVERAGE_DIR)/maxclique.txt
	gcov -o ../part7 ../part7/cliquecount.c > $(COVERAGE_DIR)/cliquecount.txt
	@echo "Coverage reports saved in $(COVERAGE_DIR)/"

html: run
	lcov --capture --directory . --output-file coverage.info
	lcov --capture --directory ../part7 --output-file coverage_part7.info
	lcov -a coverage.info -a coverage_part7.info -o total_coverage.info
	genhtml total_coverage.info --output-directory $(HTML_DIR)
	@echo "HTML report generated at $(HTML_DIR)/index.html"

clean:
	rm -f $(SERVER) $(CLIENT) $(OBJS_SERVER) $(OBJS_CLIENT) *.gcno *.gcda *.gcov coverage.info coverage_part7.info total_coverage.info
	rm -f ../part7/*.o ../part7/*.gcno ../part7/*.gcda ../part7/*.gcov
	rm -rf $(COVERAGE_DIR) $(HTML_DIR)

.PHONY: all run coverage html clean
