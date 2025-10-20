#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "crc64.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

int main(int argc, char **argv) {
    uint64_t batch_size = 8 * 1024 * 1024;
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
    
    off_t file_size = lseek(fd, 0, SEEK_END);
    size_t bytes_read, total_bytes_read = 0;
    uint64_t crc = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        printf("ERROR: mmap\n");
        close(fd);
        return 1;
    }
    for (size_t j = 0; j < file_size; j += batch_size) {
        size_t to_read;
        if (file_size - j < batch_size) to_read = file_size - j;
        else to_read = batch_size;
        crc = crc64_update(crc, (unsigned char *)(data + j), to_read);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Sequential mmap: %lX in %f seconds\n", crc, elapsed_time);
    munmap(data, file_size);
    close(fd);
}