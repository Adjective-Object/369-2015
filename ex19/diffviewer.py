#!/usr/bin/python
import sys, math

def getDiff(file1, file2):
	lineno = 0
	diffBlocks = []
	blocksize = 24
	block1 = file1.read(blocksize)
	block2 = file2.read(blocksize)

	while block1 != "" and block2 != "":
		if(block1 != block2):
			diffBlocks.append( (lineno, block1, block2) )

		lineno += 1
		block1 = file1.read(blocksize)
		block2 = file2.read(blocksize)

	return diffBlocks

	file1.close()
	file2.close()

if __name__ == "__main__":
	if len(sys.argv) != 3:
		print "Usage: diffviewer.py <file1> <file2>"
		sys.exit(1)

	diff = getDiff( open(sys.argv[1]), open(sys.argv[2]),)

	maxgap = int(max(math.log(lno,10) + 1 for (lno, x, y) in diff)) + 1

	for (line, l1, l2) in diff:
		print "%d%s%s\t%s"%(
			line,
			" "*(maxgap - int(math.log(line,10))),
			"".join("{:02x}".format(ord(c)) for c in l1), 
			"".join("{:02x}".format(ord(c)) for c in l2))