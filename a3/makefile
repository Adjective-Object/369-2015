all: debugtools  assignment

## the assignment

assignment: ext2_cp # ext2_mkdir ext2_ln ext2_rm

ext2_rm: ext2_rm.o ext2.o
	gcc -Wall -g $^ -o ext2_rm

ext2_ln: ext2_ln.o ext2.o
	gcc -Wall -g $^ -o ext2_ln

ext2_mkdir: ext2_mkdir.o ext2.o
	gcc -Wall -g $^ -o ext2_mkdir

ext2_cp: ext2_cp.o ext2.o blockgroup.o superblock.o
	gcc -Wall -g $^ -o ext2_cp

## debug stuff

debugtools: ext2_meta ext2_inspect

ext2_meta: blockgroup.o superblock.o ext2.o ext2_meta.c
	gcc -Wall -g $^ -o ext2_meta

ext2_inspect: ext2_inspect.o blockgroup.o superblock.o ext2.o
	gcc -Wall -g $^ -o ext2_inspect

## does a cool thing

%.o: %.c %.h
	gcc -Wall -g -c  $<

%.o: %.c
	gcc -Wall -g -c $<

## macros to make devel faster
.PHONY fake:

setup: fake
	-mkdir -p dump 
	-mkdir -p dump/img
	-mkdir -p dump/tst
	-mkdir -p diffs

diffs: fake
	dumpimgs.sh
	makediffs.sh

reset:
	cp img/* tst/

clean:
	rm -f *.o

## forcemacros
