assignment: ext2_cp ext2_mkdir ext2_ln ext2_rm

ext2_rm: ext2_rm.o ext2.o
	gcc -Wall -g $^ -o ext2_rm

ext2_ln: ext2_ln.o ext2.o
	gcc -Wall -g $^ -o ext2_ln

ext2_mkdir: ext2_mkdir.o ext2.o
	gcc -Wall -g $^ -o ext2_mkdir

ext2_cp: ext2_cp.o ext2.o
	gcc -Wall -g $^ -o ext2_cp


debugtools: ext2_meta

ext2_meta: ext2_meta.o superblock.o
	gcc -Wall -g $^ -o ext2_meta


%.o: %.c
	gcc -Wall -g -c  $<
