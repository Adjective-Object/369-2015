all : testmalloc

testmalloc : mymemory.o test_malloc.o
	gcc -Wall -g -o testmalloc mymemory.o test_malloc.o -lpthread

%.o: %.c
	gcc -c -o $@ $< -lpthread

clean:
	rm *.o