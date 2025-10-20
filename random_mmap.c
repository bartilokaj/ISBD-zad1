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
        printf("Usage: ./time_test file_path batch_size(optional)");
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
    
    off_t front_offset = 0;
    off_t back_offset = lseek(fd, 0, SEEK_END);
    if (back_offset < 0) {
        printf("ERROR: lseek\n");
        return 1;
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        printf("ERROR: lseek\n");
        return 1;
    }
    size_t to_read; 
    char *data = mmap(NULL, back_offset, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        printf("ERROR: mmap\n");
        close(fd);
        return 1;
    }
    uint64_t crc = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (front_offset < back_offset) {
        if (back_offset - front_offset < batch_size) to_read = back_offset - front_offset;
        else to_read = batch_size;
        crc = crc64_update(crc, data + front_offset, to_read);
        front_offset += to_read;

        if (back_offset - front_offset < batch_size) to_read = back_offset - front_offset;
        else to_read = batch_size;
        back_offset -= to_read;
        crc = crc64_update(crc, data + back_offset, to_read);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Random map: %lX in %f seconds\n", crc, elapsed_time);
    munmap(data, back_offset);
    close(fd);
}