# Compiler to use
CC = gcc

# Compiler flags to ensure strict compliance with code standards and to enable debugging
CFLAGS = -Wall -Werror -Wextra -pedantic -g -std=c99

# Additional flags for debugging
DEBUGFLAGS = -g

# Names of the target executables
TARGET_0 = hnm
TARGET_1 = hobjdump

# Source files for the hnm program
SOURCE_0 = hnm-64.c hnm-32.c hnm-main.c

# Header files for the hnm program
HEADERS_0 = hnm.h

# Source files for the hobjdump program
SOURCE_1 = hobjdump.c

# Default rule to build all programs
all: $(TARGET_0) $(TARGET_1)

# Rule to compile with debug information
debug: CFLAGS += $(DEBUGFLAGS)
debug: all

# Rule to build the hnm program
$(TARGET_0): $(SOURCE_0) $(HEADERS_0)
	$(CC) $(CFLAGS) -o $(TARGET_0) $(SOURCE_0)

# Rule to build the hobjdump program
$(TARGET_1): $(SOURCE_1)
	$(CC) $(CFLAGS) -o $(TARGET_1) $(SOURCE_1)

# Rule to clean up the compiled binaries and object files
clean:
	rm -f $(TARGET_0) $(TARGET_1)
	rm -f *.o
	rm -f *.out
	rm -f *~

# Declaring phony targets to avoid conflicts with files of the same name
.PHONY: all clean debug