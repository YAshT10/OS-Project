# Build the shared library
libhook.so: hook.c
	gcc -shared -fPIC -o libhook.so hook.c -ldl

# Build the executable
wait_for_all : main.c
	gcc -o wait_for_all main.c

# Run the executable with LD_PRELOAD
run: libhook.so wait_for_all
	LD_PRELOAD=./libhook.so ./wait_for_all

# Build everything (libhook.so and cloak)
all: libhook.so wait_for_all

# make all for make
# make run for running