# The operating system that the binary should go to: linux/windows
BIN_OS = windows

# Directories
SRC_DIR = .
OBJ_DIR = .
BIN_DIR = ../bin
INC_DIR = .

# Binary name, more defined in the ifeqs
BIN = $(BIN_DIR)/mal

# Compiler and flags
CC = g++
CFLAGS = -Wall -O3 -I$(INC_DIR) -D PARANOID -ggdb
LIBS =

# Compiling for Linux
ifeq ($(BIN_OS),linux)
	# Compiling via windows is a lost cause
	ifeq ($(OS),Windows_NT)
\	\	$(error Please compile the program with WSL, it's easiest.)
	endif
	
	CFLAGS += -Wl,-R,'$$ORIGIN'
	LIBS += -lX11 -lXrandr -lm -lpulse -lpulse-simple -lXfixes

# Otherwise compiling for Windows
else ifeq ($(BIN_OS),windows)

	# Gonna need to use mingw cross compiler
	ifeq ($(shell uname -s),Linux)
		CC = x86_64-w64-mingw32-g++
	endif
	
	CFLAGS += -static-libgcc -static-libstdc++
	LIBS += -lgdi32 -lwinmm -lmsvcrt -lmingw32 -lws2_32
	
endif

# Source files
SRC = $(wildcard $(SRC_DIR)/*.cpp)
# Add platform-specific files
SRC += $(wildcard $(SRC_DIR)/$(BIN_OS)/*.cpp)
# Object files
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# Rule to build binary
$(BIN): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Rule to compile object files, with PIC
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) -fpic $(CFLAGS) -c $< -o $@

# Create necessary directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Phony targets
.PHONY: all clean run

all: $(BIN) $(wildcard $(SRC_DIR)/*.hpp) $(SRC)

clean:
	rm -rf $(OBJS) $(BIN)

run: $(BIN)
	$(BIN) -f sanserif.psf

