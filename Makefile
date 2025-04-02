# Compiler to use
CC = gcc

# Compiler flags
# -Wall: Enable all warnings
# -Wextra: Enable extra warnings
# -g: Include debugging information
# -pthread: Link with pthread library
CFLAGS = -Wall -Wextra -g -pthread

# Source directory
SRC_DIR = src

# Output directory for object files
OBJ_DIR = obj

# Binary output directory
BIN_DIR = bin

# Target executables
DEFAULT_TARGET = $(BIN_DIR)/default_program
BATCHED_TARGET = $(BIN_DIR)/batched_program
MULTI_TARGET = $(BIN_DIR)/multi_program
NORB_TARGET = $(BIN_DIR)/norb_program

# Source files
DEFAULT_SRCS = $(SRC_DIR)/default.c \
               $(SRC_DIR)/timing.c

BATCHED_SRCS = $(SRC_DIR)/batched.c \
               $(SRC_DIR)/timing.c

MULTI_SRCS = $(SRC_DIR)/multiqueue.c \
               $(SRC_DIR)/timing.c

NORB_SRCS = $(SRC_DIR)/noqueue.c \
               $(SRC_DIR)/timing.c

# Object files
DEFAULT_OBJS = $(DEFAULT_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BATCHED_OBJS = $(BATCHED_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
MULTI_OBJS 	 = $(MULTI_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
NORB_OBJS 	 = $(NORB_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Header files
HEADERS = $(SRC_DIR)/ring_buffer.h \
          $(SRC_DIR)/timing.h

# Create directories if they don't exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Default target builds both executables
all: $(DEFAULT_TARGET) $(BATCHED_TARGET) $(MULTI_TARGET) $(NORB_TARGET)

# Rule to build the default executable
$(DEFAULT_TARGET): $(DEFAULT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build the batched executable
$(BATCHED_TARGET): $(BATCHED_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build the multiqueue executable
$(MULTI_TARGET): $(MULTI_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build the multiqueue executable
$(NORB_TARGET): $(NORB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Phony targets
.PHONY: all clean run_default run_batched run_multi run_norb debug_default debug_batched debug_multi debug_norb

# Run the default executable
run_default: $(DEFAULT_TARGET)
	$(DEFAULT_TARGET)

# Run the batched executable
run_batched: $(BATCHED_TARGET)
	$(BATCHED_TARGET)

# Run the batched executable
run_multi: $(MULTI_TARGET)
	$(MULTI_TARGET)

# Run the noqueue executable
run_norb: $(NORB_TARGET)
	$(NORB_TARGET)

# Debug the default executable with gdb
debug_default: $(DEFAULT_TARGET)
	gdb $(DEFAULT_TARGET)

# Debug the batched executable with gdb
debug_batched: $(BATCHED_TARGET)
	gdb $(BATCHED_TARGET)

# Debug the multi executable with gdb
debug_multi: $(MULTI_TARGET)
	gdb $(MULTI_TARGET)

# Debug the noqueue executable with gdb
debug_norb: $(NORB_TARGET)
	gdb $(NORB_TARGET)
