# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -I.

# Source files
SRCS = buffer.c control_permission.c logger.c rate_limiting.c Safe_open.c Safe_read.c master_wrapper.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
EXEC = master_wrapper

# Default target
all: $(EXEC)

# Link object files to create the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean