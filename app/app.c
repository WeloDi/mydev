#include "mydev.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEVICE "/dev/mydev"

int ioctl_malloc(int fd, size_t size)
{
    if (ioctl(fd, MALLOC_CMD, size) == -1) {
        perror("Failed to allocate memory in kernel space");
        return -1;
    }
    printf("malloc %zu bytes in kernel space.\n", size);
}

int ioctl_write(int fd, char *filename)
{
    char path[100] = {'\0'};
    sprintf(path, "../file/%s", filename);

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        perror("open file error");
        return 1;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // 分配内存
    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Malloc error");
        fclose(file);
        return 1;
    }

    // 读取文件内容到缓冲区
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead < fileSize) {
        perror("Reading file error");
        free(buffer);
        fclose(file);
        return 1;
    }
    buffer[fileSize] = '\0';
    size_t buffer_size = strlen(buffer) + 1;

    DATA data = {buffer, buffer_size};

    // 申请内核空间
    ioctl_malloc(fd, buffer_size);

    // 写入数据
    int ret = ioctl(fd, WRITE_CMD, &data);
    if (ret < 0) {
        perror("Failed to write data to kernel space");
    }
    printf("write %zu bytes to kernel space:\n%s\n", data.len, data.address);

    free(buffer);
    fclose(file);
    return 0;
}

int ioctl_read(int fd, char *filename, size_t size)
{
    int ret = 0;
    char *buffer = (char *)malloc(size);
    memset(buffer, '\0', size);

    DATA data = {buffer, size};

    // 读取
    ret = ioctl(fd, READ_CMD, &data);
    if (ret < 0) {
        perror("Failed to read data from kernel space");
        free(buffer);
        buffer = NULL;
        return -1;
    }

    // 写入文件
    char path[100] = {'\0'};
    sprintf(path, "../file/%s", filename);

    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        perror("Error open file");
        return -1;
    }
    fputs(buffer, fp);
    printf("write data to %s:\n%s\n", filename, buffer);

    fclose(fp);
    free(buffer);
    buffer = NULL;
    return 0;
}

size_t ioctl_getLen(int fd)
{
    size_t len = 0;
    ioctl(fd, GET_LEN, &len);
    return len;
}

int main(int argc, char const *argv[])
{
    int fd = open(DEVICE, O_RDWR);
    if (fd == -1) {
        perror("Failed to open the device");
        return -1;
    }

    int cmd;
    size_t size;
    char filename[100] = {'\0'};
    while (1) {
        printf("\n------输入命令------\n0: 申请内核内存\n1: 写入文件到内核\n2: 读取内核文件\n--------------------\n");
        scanf("%d", &cmd);
        switch (cmd) {
        case 0:
            printf("申请大小:");
            scanf("%zu", &size);
            ioctl_malloc(fd, size);
            break;
        case 1:
            printf("文件名:");
            scanf("%s", filename);
            getchar();
            ioctl_write(fd, filename);
            break;
        case 2:
            printf("文件名:");
            scanf("%s", filename);
            getchar();
            size = ioctl_getLen(fd);
            ioctl_read(fd, filename, size);
            break;
        default:
            close(fd);
            return 0;
        }
    }

    close(fd);
    return 0;
}
