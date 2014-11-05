#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys


def readMarkerFile(markerfile):
    """
    reads the contents of a given marker file if one is specified,
    otherwise, it reads the contents of ./marker.

    outputs the addresses of the beginning and end of the relavant
    areas of the trace (marer_start, marker_end)
    """
    if markerfile == "<null>":
        return "<null>", "<null>"

    with open(markerfile, 'r') as f:
        m_start, m_end = f.readline().split(" ")

    return m_start[2:], m_end[2:]


def dropComments(f, line):
    """
    Reads a series of lines from a file while they begin with the string "=="
    returns the first line not beginning with "=="

    Used to drop the section of comments from the beginning of the file.
    """
    while line.startswith("=="):
        line = f.readline();
    return line


def readBody(f, line, marker_start, marker_end):
    """
    (File, String, Int, Int) -> (Int, Int, Int, Int)

    Reads the body of a valgrind lackey trace 
    (i.e. the section between the ==comments== at the head and tail of the file)

    Called by analyzeTrace, (see analyzeTrace for return values)
    """
    # initialize counter variables
    codepages, datapages = [], []
    outsideCount = 0
    insideCount = 0
    inside = False

    while not line.startswith("=="):
        # only count when between mem markers
        # find the block with the addr
        address = line.split(" ")[-1].split(",")[0]
        pagenumber = address[0:-3]

        # exit the 'inside' section if the parser encounters the marker_end addr
        if address == marker_end:
            inside = False

        # count the number of S/L/R that happen inside/outside of the main block
        if not line.startswith("I"):
            if inside:
                insideCount += 1
            else:
                outsideCount += 1

        # manage the contents of codepages properly 
        if line.startswith("I"):
            if pagenumber not in codepages:
                codepages.append(pagenumber)

        else:
            if pagenumber not in datapages:
                datapages.append(pagenumber)


        # enter the 'inside' section if 
        # the parser encounters the marker_start addr
        if address == marker_start:
            inside = True

        # update line
        line = f.readline()

    return (len(codepages), len(datapages), 
            insideCount if marker_start != "<null>" else "",
            outsideCount if marker_start != "<null>" else "")


def analyzeTrace(tracefile, markerfile):
    """
    (String, String) -> (Int, Int, Int, Int)

    reads a valgrind lackey trace and a marker file and determines:

    I:            the number of pages from which instructions are read
    S/L/M:        the number of pages from which data is set/loaded/moved
    insideCount:  the number of instrutions between marker_start and marker_end
    outsideCount: the number of instructions not between 
                    marker_start and marker_end

    """
    # read marker from file
    marker_start, marker_end = readMarkerFile(markerfile)
    #print marker_start, marker_end

    # open file and read the first line
    f = open(tracefile, 'r')
    line = f.readline()

    # ignore the comment lines at beginning
    line = dropComments(f, line)

    # read the body of the trace
    traceData = readBody(f, line, marker_start, marker_end)

    # close the file
    f.close()

    return traceData 


def printTable(table):
    """
    ([[String]]) -> None

    prints the contents of a table, defined in a mxn 2 dimensional list, indexed
    row first.

    the first row is assumed to be headers, and the list is assumed to have
    conscistent dimensions (i.e. all rows of equal length).
    """
    # calculate columns widths
    cwidths = [0] * len(table[0])
    for row in table:
        for i in range(len(row)):
            cwidths[i] = max(len(row[i]), cwidths[i])
    #
    rowlen = sum([x+3 for x in cwidths])+ 1

    print " " + (rowlen-2) * "-" + " "


    for ind in range(len(table)):
        if(ind == 1):
           print "|" + (rowlen-2) * "-" + "|"
        row = table[ind]
        row = [row[i] + " " * 
            (cwidths[i] - len(row[i])) for i in range(len(row))]
        print "| "+" | ".join(row)+" |"

    print " " + (rowlen-2) * "-" + " "


if __name__ == "__main__":
    if len(sys.argv) %2  != 1:
        print( "analyze.py: improper args.\n"
               "usage is \"analyze.py <trace1> <marker1> "
               "<trace2> <marker2> ...\"\n"
             )
    #divide the arguments into (tracefile, markerfile) pairs
    pairs = [(sys.argv[i], sys.argv[i+1]) for i in range(1,len(sys.argv),2)]

    # Initialize the headers of the table
    table = [["Tracefile", "Markerfile", "I", "S/L/R", "Inside", "Outside"]]
    for trace, marker in pairs:
        ptrace = trace.split("/")[-1]
        pmarker = marker.split("/")[-1]
        table.append( [ptrace, pmarker]+ map(str, analyzeTrace(trace, marker)))

    printTable(table)
