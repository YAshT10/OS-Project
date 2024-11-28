# Compiler
CC = gcc

# Compiler flags
CFLAGS += -fPIC

# Source files
SRCS = buffer.c control_permission.c logger.c rate_limiting.c Safe_open.c Safe_read.c master_wrapper.c

# Object files
OBJS = $(SRCS:.c=.o)

EXEC = shared.so

test: main.c
	gcc -o test main.c

run: $(EXEC) test
	LD_PRELOAD=./shared.so ./test

# Default target
all: $(EXEC) test

# Link object files to create the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -fPIC

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)