# ================================
# Compiler and Flags Definitions
# ================================

# Compiler to use
CC          = gcc

# Compiler flags for strict code compliance and debugging
CFLAGS      = -Wall -Werror -Wextra -pedantic -g -std=c99

# Additional debug flags
DEBUGFLAGS  = -g

# ================================
# Target Executable Names
# ================================

TARGET_0    = hnm
TARGET_1    = hobjdump

# ================================
# Source and Header Files
# ================================

# Source files for the hnm program
SOURCE_0    = hnm-64.c hnm-32.c hnm-main.c

# Header files for the hnm program
HEADERS_0   = hnm.h

# Source files for the hobjdump program
SOURCE_1    = hobjdump.c

# ================================
# Build Rules
# ================================

# Default target to build all programs
all: $(TARGET_0) $(TARGET_1)

# Debug target: builds with debug flags
debug: CFLAGS += $(DEBUGFLAGS)
debug: all

# Build the hnm executable
$(TARGET_0): $(SOURCE_0) $(HEADERS_0)
	$(CC) $(CFLAGS) -o $(TARGET_0) $(SOURCE_0)

# Build the hobjdump executable
$(TARGET_1): $(SOURCE_1)
	$(CC) $(CFLAGS) -o $(TARGET_1) $(SOURCE_1)

# ================================
# Clean Rule
# ================================

# Remove all compiled binaries and temporary files
clean:
	rm -f $(TARGET_0) $(TARGET_1)
	rm -f *.o *.out *~

# ================================
# Phony Targets
# ================================

.PHONY: all clean debug
