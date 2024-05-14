PROJECT_NAME = kardia

# Directories
SRC_DIR = $(PROJECT_NAME)
OBJ_DIR = $(SRC_DIR)
BIN_DIR = $(SRC_DIR)/../bin
INC_DIR = $(SRC_DIR)

# Binary name, more defined in the ifeqs
BIN = $(BIN_DIR)/$(PROJECT_NAME)

# Compiler and flags
CC = gcc
CFLAGS = -Wall -O3 -L$(BIN_DIR) -I$(INC_DIR) -I$(SRC_DIR)/../engine -ggdb -Wl,-R,'$$ORIGIN'
LIBS = -lengine

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)
# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Rule to build binary
$(BIN): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Rule to compile object files, with PIC
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Phony targets
.PHONY: all clean run

all: $(BIN)

clean:
	rm -f -I $(OBJS)

