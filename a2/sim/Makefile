
sim :  sim.o pagetable.o avl.o rand.o clock.o lru.o fifo.o opt.o inthashmap.o
	gcc -Wall -g -o sim $^

%.o : %.c avl.h pagetable.h
	gcc -Wall -g -c $<

clean : 
	rm *.o sim

tables : sim tablemaker.py
	python ./tablemaker.py \
		-f a2-traces/simpleloop a2-traces/matmul-100 a2-traces/blocked-100-25 \
		-m 50 100 150 200 \
		-a fifo > tables
	
	echo >> tables
	python ./tablemaker.py \
		-f a2-traces/simpleloop a2-traces/matmul-100 a2-traces/blocked-100-25 \
		-m 50 100 150 200 \
		-a lru >> tables
	
	echo >> tables
	python ./tablemaker.py \
		-f a2-traces/simpleloop a2-traces/matmul-100 a2-traces/blocked-100-25 \
		-m 50 100 150 200 \
		-a clock >> tables
	
	echo >> tables
	python ./tablemaker.py \
		-f a2-traces/simpleloop a2-traces/matmul-100 a2-traces/blocked-100-25 \
		-m 50 100 150 200 \
		-a opt >> tables

clock :
	python ./tablemaker.py \
		-f a2-traces/simpleloop a2-traces/matmul-100 a2-traces/blocked-100-25 \
		-m 50 100 150 200 \
		-a clock
