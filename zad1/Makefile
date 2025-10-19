all:
	gcc -c crc64.c -o crc64.o
	gcc -c time_test.c -o time_test.o
	gcc time_test.o crc64.o -o time_test

clean:
	rm -f *.o
	rm -f time_test