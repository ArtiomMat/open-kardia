# The operating system that the binary should go to: nix/win/mac/bsd
BIN_OS = nix

# Compiler and flags
CC = gcc
CFLAGS = -Wall -O3 -Iinc -ggdb -D DEBUG

ifeq ($(BIN_OS),nix)
	# Libraries
	LIBS = -lX11 -lXrandr -lm -lpulse -lpulse-simple
else ifeq ($(BIN_OS),win)
	#TODO
	LIBS = -lgdi32
endif

# Source directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)
SRC += $(wildcard $(SRC_DIR)/$(BIN_OS)/*.c) # Add platform-specific files
# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Binary
BIN = $(BIN_DIR)/kardia

# Rule to build binary
$(BIN): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Rule to compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/$(PLATFORM)

# Phony targets
.PHONY: all clean run

all: $(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: $(BIN)
	$(BIN) -f sanserif.psf
