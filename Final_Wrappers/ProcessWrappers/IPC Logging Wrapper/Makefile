#Change the file name to suit your needs /tmp/ works as well
# This Makefile builds the shared library and the executable
# with the shared library preloaded.
# To build the shared library and the executable cloak, run: make cloak
# To run the executable with the shared library preloaded for cloak, run: make runcloak
# Build the shared library
libhook.so: hook.c
	gcc -shared -fPIC -o libhook.so hook.c -ldl

libipchook.so: ipchook.c
	gcc -shared -fPIC -o libipchook.so ipchook.c -ldl

libdeadlockhook.so: deadlockhook.c
	gcc -shared -fPIC -o libdeadlockhook.so deadlockhook.c -ldl -lpthread

# Build the executable
cloak: cloak.c
	gcc -o cloak cloak.c

test_ipc: ipc.c
	gcc -o test_ipc ipc.c

deadlock: deadlock.c
	gcc -o deadlock deadlock.c

# Run the executable with LD_PRELOAD
runcloak: libhook.so cloak
	LD_PRELOAD=./libhook.so ./cloak

runipc: libipchook.so test_ipc
	LD_PRELOAD=./libipchook.so ./test_ipc 

rundeadlock: libdeadlockhook.so deadlock
	LD_PRELOAD=./libdeadlockhook.so ./deadlock
