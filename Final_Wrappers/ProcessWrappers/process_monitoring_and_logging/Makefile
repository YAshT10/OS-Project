# Build the shared library
libhook.so: hook.c
	gcc -shared -fPIC -o libhook.so hook.c -ldl

# Build the executable
monitor_and_log : main.c
	gcc -o monitor_and_log main.c

# Run the executable with LD_PRELOAD
run: libhook.so monitor_and_log
	LD_PRELOAD=./libhook.so ./monitor_and_log

# Build everything (libhook.so and cloak)
all: libhook.so monitor_and_log

# make all for make
# make run for running