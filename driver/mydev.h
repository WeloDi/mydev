#ifndef MYDEV_H
#define MYDEV_H
#include <linux/ioctl.h>
#include <stddef.h>

typedef struct data {
    char *address;
    size_t len;
} DATA;

#define MALLOC_CMD _IOW('m', 0, size_t)
#define WRITE_CMD _IOW('m', 1, DATA *)
#define READ_CMD _IOR('m', 2, DATA *)
#define GET_LEN _IOR('m', 3, size_t *)
#endif