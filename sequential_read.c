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
    
    char *buf = malloc(batch_size);
    size_t bytes_read;
    size_t total_bytes_read = 0;
    uint64_t crc = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while ((bytes_read = read(fd, buf, batch_size)) > 0) {
        crc = crc64_update(crc, buf, bytes_read);
    }
    if (bytes_read < 0) {
        printf("ERROR: read\n");
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Sequential read: %lX in %f seconds\n", crc, elapsed_time);
    free(buf);
    close(fd);
}