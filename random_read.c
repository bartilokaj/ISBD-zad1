#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "crc64.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

int main(int argc, char **argv) {
    uint64_t batch_size = 8 * 1024 * 1024;
    uint64_t memory_size = 8UL * 1024 * 1024 * 1024;
    if (argc < 2) {
        printf("Usage: ./time_test file_path batch_size(optional) memory_size(optional)");
        return 1;
    }
    if (argc > 2) {
        batch_size = atoi(argv[2]);
    } 
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("ERROR: open\n");
        return 1;
    }
    
    char *buf = malloc(batch_size);
    if (buf == NULL) {
        printf("ERROR: malloc\n");
        return 1;
    }
    size_t bytes_read;
    uint64_t crc = 0;
    off_t front_offset = 0;
    off_t back_offset = lseek(fd, 0, SEEK_END);
    if (back_offset < 0) {
        printf("ERROR: lseek\n");
        free(buf);
        return 1;
    }
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (front_offset < back_offset) {
        size_t to_read;
        if (back_offset - front_offset < batch_size) to_read = back_offset - front_offset;
        else to_read = batch_size;
        lseek(fd, front_offset, SEEK_SET);
        if (lseek < 0) {
            printf("ERROR: lseek\n");
            free(buf);
            return 1;
        }
        bytes_read = read(fd, buf, to_read);
        if (bytes_read < 0) {
            printf("ERROR: read\n");
            free(buf);
            return 1;
        }
        crc = crc64_update(crc, buf, bytes_read);
        if (bytes_read <= 0) break;
        front_offset += bytes_read;

        if (front_offset >= back_offset) break;

        if (back_offset - front_offset < batch_size) to_read = back_offset - front_offset;
        else to_read = batch_size;
        back_offset -= to_read;
        lseek(fd, back_offset, SEEK_SET);
        if (lseek < 0) {
            printf("ERROR: lseek\n");
            free(buf);
            return 1;
        }
        bytes_read = read(fd, buf, to_read);
        if (bytes_read < 0) {
            printf("ERROR: read\n");
            free(buf);
            return 1;
        }
        if (bytes_read == 0) break;
        crc = crc64_update(crc, buf, bytes_read);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Random read: %lX in %f seconds\n", crc, elapsed_time);
    free(buf);
    close(fd);
}