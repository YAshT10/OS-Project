#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "logger.h"
#include "Safe_open.h"

void check_path(const char* path) {
    char *temp;
    char *p = NULL;
    size_t len;

    temp = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!temp) {
        printk(KERN_ERR "Error: Memory allocation failed\n");
        return;
    }

    snprintf(temp, PATH_MAX, "%s", path);
    len = strlen(temp);

    if (temp[len - 1] == '/') {
        temp[len - 1] = 0;
    }

    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (kern_path(temp, LOOKUP_FOLLOW, NULL) != 0) {
                printk(KERN_ERR "Error: Directory %s doesn't exist\n", temp);
                kfree(temp);
                return;
            }
            *p = '/';
        }
    }

    if (kern_path(temp, LOOKUP_FOLLOW, NULL) != 0) {
        printk(KERN_ERR "Error: File %s doesn't exist\n", temp);
        kfree(temp);
        return;
    }

    kfree(temp);
}

int safe_open(const char* filename, int flags, umode_t mode) {
    struct file *file;
    mm_segment_t old_fs;
    int err;

    // If mode is "r", check if the file exists
    if (flags == O_RDONLY && kern_path(filename, LOOKUP_FOLLOW, NULL) != 0) {
        check_path(filename);
        return -ENOENT;
    }

    if ((flags == O_WRONLY || flags == O_RDWR) && kern_path(filename, LOOKUP_FOLLOW, NULL) != 0) {
        printk(KERN_INFO "File %s doesn't exist; Do you want to create a new file? Y/N\n", filename);
        // In kernel space, we cannot interact with the user directly
        // Assuming the user wants to create the file
        file = filp_open(filename, O_CREAT | O_WRONLY, mode);
        if (IS_ERR(file)) {
            printk(KERN_ERR "Error creating file\n");
            return PTR_ERR(file);
        }
        filp_close(file, NULL);
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    file = filp_open(filename, flags, mode);
    set_fs(old_fs);

    if (IS_ERR(file)) {
        printk(KERN_ERR "Error opening file\n");
        return PTR_ERR(file);
    }

    filp_close(file, NULL);
    return 0;
}