# Implementing Customized Logging Wrappers for Linux File I/O System Calls

This guide describes the steps to create custom wrappers around file I/O system calls (`open`, `read`, `write`, `close`) in the Linux kernel to log information about file access.

## Prerequisites
1. **Kernel Development Setup**: Ensure you have a kernel development environment set up.
2. **Kernel Headers**: Install the necessary kernel headers.
3. **Loadable Kernel Module (LKM)**: Familiarity with creating and compiling LKMs.
4. **Root Privileges**: Root access is required to load modules and modify kernel system calls.

---

## Step 1: Create a New Kernel Module
1. **Set up a directory** for the kernel module, e.g., `mkdir file_io_logger`.
2. **Navigate to the directory**: `cd file_io_logger`.
3. **Create the source file** for the module:
   ```bash
   touch file_io_logger.c

## Step 2: Define the Logging Logic
Open `file_io_logger.c` in your preferred editor.

Include necessary headers:

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/time.h>
```

Define a structure to store original system calls:

```c
static asmlinkage long (*original_open)(const char __user *, int, umode_t);
static asmlinkage ssize_t (*original_read)(int, void __user *, size_t);
static asmlinkage ssize_t (*original_write)(int, const char __user *, size_t);
static asmlinkage long (*original_close)(unsigned int);
```

Define the log function to save file access details:

```c
void log_file_access(const char *operation, int fd, ssize_t bytes) {
    struct timeval time;
    do_gettimeofday(&time);
    printk(KERN_INFO "[FileIOLogger] %s | FD: %d | Bytes: %zd | Timestamp: %ld.%ld\n",
           operation, fd, bytes, time.tv_sec, time.tv_usec);
}
```

## Step 3: Implement Custom Wrappers
Override the open system call:

```c
static asmlinkage long custom_open(const char __user *filename, int flags, umode_t mode) {
    long fd = original_open(filename, flags, mode);
    log_file_access("open", fd, 0);
    return fd;
}
```

Override the read system call:

```c
static asmlinkage ssize_t custom_read(int fd, void __user *buf, size_t count) {
    ssize_t bytes = original_read(fd, buf, count);
    log_file_access("read", fd, bytes);
    return bytes;
}
```

Override the write system call:

```c
static asmlinkage ssize_t custom_write(int fd, const char __user *buf, size_t count) {
    ssize_t bytes = original_write(fd, buf, count);
    log_file_access("write", fd, bytes);
    return bytes;
}
```

Override the close system call:

```c
static asmlinkage long custom_close(unsigned int fd) {
    log_file_access("close", fd, 0);
    return original_close(fd);
}
```

## Step 4: Replace System Calls with Wrappers
Use `kallsyms_lookup_name` (ensure kernel version supports it) to get addresses of original system calls.

Replace system calls in the `sys_call_table`:

```c
// Replace with custom_open, custom_read, custom_write, custom_close
```

Restore system calls on exit.

## Step 5: Build and Test the Module
Create a Makefile to build the module:

```makefile
obj-m += file_io_logger.o
all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

Compile the module:

```bash
make
```

Insert the module:

```bash
sudo insmod file_io_logger.ko
```

Check `dmesg` for logs:

```bash
dmesg | grep "[FileIOLogger]"
```

## Step 6: Clean Up
Remove the module:

```bash
sudo rmmod file_io_logger
```

Revert any changes to the `sys_call_table` to restore system integrity.