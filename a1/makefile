all: safe unsafe

safe: testmalloc testmalloc_opt

unsafe: testmalloc_unsafe testmalloc_opt_unsafe

testmalloc: mymemory.o test_malloc.o malloc_structs.h
	gcc -Wall -g -o testmalloc mymemory.o test_malloc.o -lpthread

testmalloc_opt: mymemory_opt.o test_malloc.o malloc_structs.h
	gcc -Wall -I./ -g -o testmalloc_opt mymemory_opt.o test_malloc.o -lpthread

%.o: %.c
	gcc -c -o $@ $< -lpthread

clean:
	rm *.o testmalloc testmalloc_opt testmalloc_unsafe testmalloc_opt_unsafe

testmalloc*: malloc_structs.h
