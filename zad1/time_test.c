#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "crc64.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

#define CHUNK_SIZE 8 * 1024 * 1024

void sequential_read(int fd);
void random_read(int fd);
void sequential_mmap(int fd);
void random_mmap(int fd);

int main(int argc, char **argv) {
    int fd = open(argv[1], O_RDONLY);
    sequential_read(fd);
    random_read(fd);
    sequential_mmap(fd);
    random_mmap(fd);
    close(fd);
}

void sequential_read(int fd) {
    char *buf = malloc(CHUNK_SIZE);
    size_t bytes_read;
    size_t total_bytes_read = 0;
    uint64_t crc = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while ((bytes_read = read(fd, buf, CHUNK_SIZE)) > 0) {
        crc = crc64_update(crc, buf, bytes_read);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Sequential read: %lX in %f seconds\n", crc, elapsed_time);
    free(buf);
}

void random_read(int fd) {
    char *buf = malloc(CHUNK_SIZE);
    size_t bytes_read;
    uint64_t crc = 0;
    off_t front_offset = 0;
    off_t back_offset = lseek(fd, 0, SEEK_END);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (front_offset < back_offset) {
        size_t to_read;
        if (back_offset - front_offset < CHUNK_SIZE) to_read = back_offset - front_offset;
        else to_read = CHUNK_SIZE;
        lseek(fd, front_offset, SEEK_SET);
        bytes_read = read(fd, buf, to_read);
        crc = crc64_update(crc, buf, bytes_read);
        if (bytes_read <= 0) break;
        front_offset += bytes_read;

        if (front_offset >= back_offset) break;

        if (back_offset - front_offset < CHUNK_SIZE) continue;
        else to_read = CHUNK_SIZE;
        back_offset -= to_read;
        lseek(fd, back_offset, SEEK_SET);
        bytes_read = read(fd, buf, to_read);
        crc = crc64_update(crc, buf, bytes_read);
        if (bytes_read <= 0) break;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Random read: %lX in %f seconds\n", crc, elapsed_time);
    free(buf);
}

void sequential_mmap(int fd) {
    off_t file_size = lseek(fd, 0, SEEK_END);
    char *data;
    size_t bytes_read, total_bytes_read = 0;
    uint64_t crc = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (off_t i = 0; i < file_size; i += CHUNK_SIZE) {
        size_t to_read;
        if (file_size - i < CHUNK_SIZE) to_read = file_size - i;
        else to_read = CHUNK_SIZE;
        data = mmap(NULL, to_read, PROT_READ, MAP_PRIVATE, fd, i);
        if (data == MAP_FAILED) {
            printf("fail");
        }
        crc = crc64_update(crc, data, to_read);
        munmap(data, to_read);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Sequential mmap: %lX in %f seconds\n", crc, elapsed_time);
}

void random_mmap(int fd) {
    off_t front_offset = 0;
    off_t back_offset = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    size_t to_read; 
    char *data;
    uint64_t crc = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (front_offset < back_offset) {
        if (back_offset - front_offset < CHUNK_SIZE) to_read = back_offset - front_offset;
        else to_read = CHUNK_SIZE;
        data = mmap(NULL, to_read, PROT_READ, MAP_PRIVATE, fd, front_offset);
        front_offset += to_read;
        crc = crc64_update(crc, data, to_read);
        munmap(data, to_read);

        if (back_offset - front_offset < CHUNK_SIZE) to_read = back_offset - front_offset;
        else to_read = CHUNK_SIZE;
        back_offset -= to_read;
        data = mmap(NULL, to_read, PROT_READ, MAP_PRIVATE, fd, back_offset);
        crc = crc64_update(crc, data, to_read);
        munmap(data, to_read);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Random map: %lX in %f seconds\n", crc, elapsed_time);
}