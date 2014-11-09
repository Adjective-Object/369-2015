
sim :  sim.o pagetable.o avl.o rand.o clock.o lru.o fifo.o opt.o inthashmap.o
	gcc -Wall -g -o sim $^

%.o : %.c avl.h pagetable.h
	gcc -Wall -g -c $<

clean : 
	rm *.o sim

table :
	python ./tablemaker.py -f a2-traces/simpleloop a2-traces/matmul-100 a2-traces/blocked-100-25 -m 50 -a fifo lru clock opt
