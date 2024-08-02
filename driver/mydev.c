#include "mydev.h"
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#define DEV_NAME "mydev"

struct KERNEL_DATA {
    char *address;
    size_t capacity;
    size_t len;
};

static struct miscdevice mydev_misc = {
    .minor = MISC_DYNAMIC_MINOR, // 自动分配次设备号
    .name = DEV_NAME,
};

struct KERNEL_DATA kernel_data = {
    .address = NULL,
    .capacity = 0,
    .len = 0,
};

static int mydev_open(struct inode *inode, struct file *file)
{
    printk("mydev_open is called\n");
    return 0;
}

static int mydev_close(struct inode *inode, struct file *file)
{
    if (kernel_data.address) {
        vfree(kernel_data.address);
        kernel_data.address = NULL;
    }
    printk("mydev_close is called\n");
    return 0;
}

static long mydev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    size_t size = 0;
    DATA __user user_data; // 保存用户参数
    long ret = 0;

    switch (cmd) {
    case MALLOC_CMD:
        if (kernel_data.address) {
            vfree(kernel_data.address);
        }

        size = (size_t)arg;

        kernel_data.address = (char *)vmalloc(size);
        if (!kernel_data.address) {
            ret = -ENOMEM;
            break;
        }
        memset(kernel_data.address, 0, size);
        kernel_data.capacity = size;
        kernel_data.len = 0;
        printk(KERN_INFO "Allocated %zu bytes of kernel memory\n", kernel_data.capacity);
        break;

    case WRITE_CMD:
        // 保存参数
        if (copy_from_user(&user_data, (DATA __user *)arg, sizeof(DATA))) {
            ret = -EFAULT;
            break;
        }

        // 条件判断
        if (kernel_data.address == NULL || user_data.len > kernel_data.capacity) {
            ret = -EOVERFLOW;
            break;
        }

        // 写入
        if (copy_from_user(kernel_data.address, user_data.address, user_data.len)) {
            ret = -EFAULT;
            break;
        }
        kernel_data.len = user_data.len;

        printk(KERN_INFO "Wrote %zu bytes to kernel memory:\n%s\n", kernel_data.len, kernel_data.address);
        break;

    case READ_CMD:
        // 保存参数
        if (copy_from_user(&user_data, (DATA __user *)arg, sizeof(DATA))) {
            ret = -EFAULT;
            break;
        }

        // 条件判断
        if (kernel_data.address == NULL || user_data.len > kernel_data.len) {
            ret = -EOVERFLOW;
            break;
        }

        // 写出
        if (copy_to_user(user_data.address, kernel_data.address, user_data.len)) {
            ret = -EFAULT;
            break;
        }

        printk(KERN_INFO "Read %zu bytes from kernel memory\n", user_data.len);
        break;
    case GET_LEN:
        if (copy_to_user((size_t __user *)arg, &kernel_data.len, sizeof(size_t))) {
            ret = -EFAULT;
            break;
        }
        break;
    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

static struct file_operations mydev_fops = {
    .owner = THIS_MODULE,
    .open = mydev_open,
    .release = mydev_close,
    .unlocked_ioctl = mydev_ioctl,
};

static int __init mydev_init(void)
{
    int ret = 0;

    mydev_misc.fops = &mydev_fops;
    ret = misc_register(&mydev_misc); // 注册设备

    return ret;
}

static void __exit mydev_exit(void)
{
    misc_deregister(&mydev_misc);
}

module_init(mydev_init);
module_exit(mydev_exit);

MODULE_AUTHOR("DengTao");
MODULE_LICENSE("GPL");
