# Build the shared library
libhook.so: hook.c
	gcc -shared -fPIC -o libhook.so hook.c -ldl

# Build the executable
logger : main.c
	gcc -o logger main.c

# Run the executable with LD_PRELOAD
run: libhook.so logger
	LD_PRELOAD=./libhook.so ./logger

# Build everything (libhook.so and cloak)
all: libhook.so logger

# make all for make
# make run for running