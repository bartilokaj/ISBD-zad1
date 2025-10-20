all:
	gcc -O3 -c crc64.c -o crc64.o
	gcc -O3 -c sequential_mmap.c -o sequential_mmap.o
	gcc -O3 -c random_mmap.c -o random_mmap.o
	gcc -O3 -c sequential_read.c -o sequential_read.o
	gcc -O3 -c random_read.c -o random_read.o
	gcc -O3 crc64.o sequential_mmap.o -o sequential_mmap
	gcc -O3 crc64.o random_mmap.o -o random_mmap
	gcc -O3 crc64.o sequential_read.o -o sequential_read
	gcc -O3 crc64.o random_read.o -o random_read
clean:
	rm -f *.o
	rm -f sequential_mmap
	rm -f random_mmap
	rm -f sequential_read
	rm -f random_read