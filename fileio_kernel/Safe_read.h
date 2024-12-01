#ifndef SAFE_READ_H
#define SAFE_READ_H

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>

ssize_t safe_read(int fd, void *buf, size_t count);

#endif // SAFE_READ_H
