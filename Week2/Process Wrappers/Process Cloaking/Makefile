# Build the shared library
libhook.so: hook.c
	gcc -shared -fPIC -o libhook.so hook.c -ldl

# Build the executable
cloak: main.c
	gcc -o cloak main.c

# Run the executable with LD_PRELOAD
run: libhook.so cloak
	LD_PRELOAD=./libhook.so ./cloak

# Build everything (libhook.so and cloak)
all: libhook.so cloak

# make all for make
# make run for running