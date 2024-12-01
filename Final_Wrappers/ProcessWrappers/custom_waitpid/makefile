# Build the shared library
libhook.so: hook.c
	gcc -shared -fPIC -o libhook.so hook.c -ldl

# Build the executable
custom_waitpid : main.c
	gcc -o custom_waitpid main.c -ldl

# Run the executable with LD_PRELOAD
run: libhook.so custom_waitpid
	LD_PRELOAD=./libhook.so ./custom_waitpid

# Build everything (libhook.so and cloak)
all: libhook.so custom_waitpid

# make all for make
# make run for running