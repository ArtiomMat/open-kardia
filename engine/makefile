# The operating system that the binary should go to: linux/windows
BIN_OS = linux

# Directories
SRC_DIR = .
OBJ_DIR = .
BIN_DIR = ../bin
INC_DIR = .

# Binary name, more defined in the ifeqs
BIN =

# Compiler and flags
CC = gcc
CFLAGS = -Wall -O3 -I$(INC_DIR) -ggdb -D NODEBUG -Wl,-rpath=$$ORIGIN
LIBS =

# Compiling for Linux
ifeq ($(BIN_OS),linux)
	# Compiling via windows is a lost cause
	ifeq ($(OS),Windows_NT)
$(error Please compile the program with WSL, it's easiest.)
	endif
	
	LIBS += -lX11 -lXrandr -lm -lpulse -lpulse-simple
	BIN = $(BIN_DIR)/libengine.so
	
# Otherwise compiling for Windows
else ifeq ($(BIN_OS),windows)

	# Gonna need to use mingw cross compiler
	ifeq ($(shell uname -s),Linux)
		CC = x86_64-w64-mingw32-gcc
	endif
	
	LIBS += -lgdi32
	
	BIN = $(BIN_DIR)/engine.dll
endif

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)
# Add platform-specific files
SRC += $(wildcard $(SRC_DIR)/$(BIN_OS)/*.c)
# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Rule to build binary
$(BIN): $(OBJS) | $(BIN_DIR)
	$(CC) -shared $(CFLAGS) $^ -o $@ $(LIBS)

# Rule to compile object files, with PIC
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -fpic $(CFLAGS) -c $< -o $@

# Create necessary directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Phony targets
.PHONY: all clean run

all: $(BIN)

clean:
	rm -rf $(OBJS) $(BIN)

run: $(BIN)
	$(BIN) -f sanserif.psf

