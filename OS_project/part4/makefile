CC       := gcc
CFLAGS   := -std=c11 -g -O0 -Wall -Wextra -Wpedantic -I../part2
LDFLAGS  :=

# --- Project ---
TARGET        := random_realloc
TARGET_GPROF  := random_realloc_gprof
TARGET_COV    := random_realloc_cov

SRC           := ../part3/random.c ../part2/graph.c
OBJ           := $(patsubst %.c,%.o,$(SRC))
OBJ_GPROF     := $(patsubst %.c,%.gprof.o,$(SRC))
OBJ_COV       := $(patsubst %.c,%.cov.o,$(SRC))

# --- Output dirs ---
VG_DIR   := valgrind
MEM_DIR  := $(CURDIR)/$(VG_DIR)/memcheck
CG_DIR   := $(CURDIR)/$(VG_DIR)/callgraph
GP_DIR   := $(CURDIR)/gprof
COV_DIR  := $(CURDIR)/coverage

# --- Run args ---
RUN_ARGS_SMALL := -v 5 -e 10 -r 42
RUN_ARGS_MED   := -v 5 -e 20 -r 42
RUN_ARGS_NO_R  := -v 5 -e 20

# --- Valgrind presets ---
VALGRIND_MEMCHECK := valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
                     --track-origins=yes --error-exitcode=1

# --- Coverage flags ---
CFLAGS_COV   := $(CFLAGS) --coverage
LDFLAGS_COV  := --coverage

.PHONY: all dirs valgrind valgrindCallGraph gprof coverage clean

# --- Build all ---
all: dirs $(TARGET)

# --- Create dirs ---
dirs:
	@mkdir -p $(MEM_DIR) $(CG_DIR) $(GP_DIR) $(COV_DIR)

# --- Link target (regular) ---
$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

# --- Build rules (regular) ---
../part3/%.o: ../part3/%.c
	$(CC) $(CFLAGS) -c $< -o $@

../part2/%.o: ../part2/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# =========================
# Valgrind: Memcheck
# =========================
valgrind: dirs $(TARGET)
	@rm -f $(CURDIR)/valgrind_report*.txt
	-$(VALGRIND_MEMCHECK) --log-file=$(MEM_DIR)/valgrind_report1.txt ./$(TARGET) $(RUN_ARGS_SMALL)
	-$(VALGRIND_MEMCHECK) --log-file=$(MEM_DIR)/valgrind_report2.txt ./$(TARGET) $(RUN_ARGS_MED)
	-$(VALGRIND_MEMCHECK) --log-file=$(MEM_DIR)/valgrind_report3.txt ./$(TARGET) $(RUN_ARGS_NO_R)
	@echo "Memcheck logs in: $(MEM_DIR)/"

# =========================
# Valgrind: Callgrind (only callgrind.out)
# =========================
valgrindCallGraph: dirs $(TARGET)
	@rm -f $(CG_DIR)/callgrind.out
	-valgrind --tool=callgrind --callgrind-out-file=$(CG_DIR)/callgrind.out \
		./$(TARGET) $(RUN_ARGS_SMALL)
	@echo "Callgrind output: $(CG_DIR)/callgrind.out"

# =========================
# gprof Profiling -> gprof/profiling.txt
# =========================
$(TARGET_GPROF): $(OBJ_GPROF)
	$(CC) $(OBJ_GPROF) $(LDFLAGS) -pg -o $@

../part3/%.gprof.o: ../part3/%.c
	$(CC) $(CFLAGS) -pg -c $< -o $@

../part2/%.gprof.o: ../part2/%.c
	$(CC) $(CFLAGS) -pg -c $< -o $@

gprof: dirs $(TARGET_GPROF)
	./$(TARGET_GPROF) $(RUN_ARGS_SMALL)
	gprof $(TARGET_GPROF) gmon.out > $(GP_DIR)/profiling.txt
	@rm -f gmon.out
	@echo "gprof output: $(GP_DIR)/profiling.txt"

# =========================
# Coverage (gcov)
# =========================
$(TARGET_COV): $(OBJ_COV)
	$(CC) $(OBJ_COV) $(LDFLAGS_COV) -o $@

../part3/%.cov.o: ../part3/%.c
	$(CC) $(CFLAGS_COV) -c $< -o $@

../part2/%.cov.o: ../part2/%.c
	$(CC) $(CFLAGS_COV) -c $< -o $@

coverage: dirs $(TARGET_COV)
	# מריצים כמה תרחישים כדי לייצר קבצי .gcda
	./$(TARGET_COV) $(RUN_ARGS_SMALL) || true
	./$(TARGET_COV) $(RUN_ARGS_MED)   || true
	./$(TARGET_COV) $(RUN_ARGS_NO_R)  || true

	# מפיקים דוחות gcov לשני הקבצים ומעבירים לתיקייה coverage/
	@gcov -b -c -o ../part2 ../part2/graph.c  >/dev/null
	@gcov -b -c -o ../part3 ../part3/random.c >/dev/null
	@mv -f graph.c.gcov  $(COV_DIR)/graph.c.gcov  || true
	@mv -f random.c.gcov $(COV_DIR)/random.c.gcov || true
	@echo "gcov reports: $(COV_DIR)/graph.c.gcov , $(COV_DIR)/random.c.gcov"

# =========================
# Clean
# =========================
clean:
	rm -f $(TARGET) $(OBJ)
	rm -f $(TARGET_GPROF) $(OBJ_GPROF)
	rm -f $(TARGET_COV) $(OBJ_COV)
	rm -rf $(VG_DIR) $(GP_DIR) $(COV_DIR)
	rm -f $(CURDIR)/valgrind_report*.txt gmon.out
