#!/bin/python
import sys, subprocess

def printTable(table):
    """
    ([[String]]) -> None

    prints the contents of a table, defined in a mxn 2 dimensional list, indexed
    row first.

    the first row is assumed to be headers, and the list is assumed to have
    conscistent dimensions (i.e. all rows of equal length).

    rows with contents as None are horizontal strikes.
    """
    # calculate columns widths
    cwidths = [0] * len(table[0])
    for row in table:
        if row:
		for i in range(len(row)):
        	    cwidths[i] = max(len(row[i]), cwidths[i])
    #
    rowlen = sum([x+3 for x in cwidths])+ 1

    print " " + (rowlen-2) * "-" + " "


    for row in table:
        if(row == None):
           print "|" + (rowlen-2) * "-" + "|"
        else:
        	row = [row[i] + " " * 
            		(cwidths[i] - len(row[i])) for i in range(len(row))]
       	 	print "| "+" | ".join(row)+" |"

    print " " + (rowlen-2) * "-" + " "

def dosim(fil, mem, alg):
	execstr = ["./sim", 
			"-f", fil, 
			"-m", mem, 
			"-a", alg]
	print execstr
	sub = subprocess.Popen(execstr, stdout=subprocess.PIPE)
	out, err = sub.communicate()

	hitcount, misscount, totalref, hitrate, missrate = ("?")*5

	for line in out.split("\n"):
		s = line.split(":");
		if s[0] == "Hit count":
			hitcount = s[1]	
		elif s[0] == "Miss count":
			misscount = s[1]
		elif s[0] == "Total references ":
			totalref = s[1]
		elif s[0] == "Hit rate":
			hitrate = s[1]
		elif s[0] == "Miss rate":
			missrate = s[1]
	return [alg, mem, fil, hitcount, misscount, totalref, hitrate, missrate]

if __name__ == "__main__": 

	args = {
		'f': [],
		'm': [],
		'a': []
	}

	curkey = None
	for arg in sys.argv[1:]:
		if arg.startswith("-"):
			curkey = arg.replace("-","")[0]
		elif curkey:
			args[curkey].append(arg)
		else:
			print "do ur args better"
			sys.exit()
	
	print args

	# Initialize the headers of the table
	table = [["algorithm", "m", "trace", "Hits", "Misses", "Total", "Hit Rate", "Miss Rate"]]

	for alg in args["a"]:
		table.append(None)
		for fil in args["f"]:
			for mem in args["m"]:
				table.append(dosim(fil, mem, alg))
			
	printTable(table)
