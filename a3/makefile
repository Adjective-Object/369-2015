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

debugtools: ext2_meta ext2_inspect ext2_traverse

ext2_meta: blockgroup.o superblock.o ext2.o ext2_meta.c
	gcc -Wall -g $^ -o ext2_meta

ext2_inspect: ext2_inspect.o blockgroup.o superblock.o ext2.o
	gcc -Wall -g $^ -o ext2_inspect

ext2_traverse: ext2_traverse.o blockgroup.o superblock.o ext2.o
	gcc -Wall -g $^ -o ext2_traverse

## does a cool thing

%.o: %.c %.h
	gcc -Wall -g -c  $<

%.o: %.c
	gcc -Wall -g -c $<

## macros to make devel faster
.PHONY fake:

setup: fake
	-mkdir -p img
	-mkdir -p tst
	-mkdir -p dump 
	-mkdir -p dump/img
	-mkdir -p dump/tst
	-mkdir -p diffs
	make wget
	make diffs

wget: fake
	wget -O img/0-emptydisk.img http://www.cdf.toronto.edu/~csc369h/fall/assignments/a3/emptydisk.img
	wget -O img/1-onefile.img http://www.cdf.toronto.edu/~csc369h/fall/assignments/a3/onefile.img
	wget -O img/2-deletedfile.img http://www.cdf.toronto.edu/~csc369h/fall/assignments/a3/deletedfile.img
	wget -O img/3-onedirectory.img http://www.cdf.toronto.edu/~csc369h/fall/assignments/a3/onedirectory.img
	wget -O img/4-hardlink.img http://www.cdf.toronto.edu/~csc369h/fall/assignments/a3/hardlink.img
	wget -O img/5-deleteddirectory.img http://www.cdf.toronto.edu/~csc369h/fall/assignments/a3/deleteddirectory.img
	

diffs: fake
	./dumpimgs.sh
	./makediffs.sh

reset:
	cp img/* tst/

clean:
	rm -f *.o

## forcemacros
