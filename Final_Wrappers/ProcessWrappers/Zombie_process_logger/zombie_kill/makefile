# Build the shared library
libhook.so: hook.c
	gcc -shared -fPIC -o libhook.so hook.c -ldl

# Build the executable
logger_kill : main.c
	gcc -o logger_kill main.c -ldl

# Run the executable with LD_PRELOAD
run: libhook.so logger_kill
	LD_PRELOAD=./libhook.so ./logger_kill

# Build everything (libhook.so and cloak)
all: libhook.so logger_kill

# make all for make
# make run for running