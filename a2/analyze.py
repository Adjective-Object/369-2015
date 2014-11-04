#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

def readMarkerFile(argv):
    """
    reads the contents of a given marker file if one is specified,
    otherwise, it reads the contents of ./marker.

    outputs the addresses of the beginning and end of the relavant
    areas of the trace (marer_start, marker_end)
    """

    mf = mf = sys.argv[2] if (len(argv) == 3) else "./marker"

    with open(mf, 'r') as f:
        m_start, m_end = f.readline().split(" ")

    return m_start[2:], m_end[2:]

def isDataPage(line):
    return (line.startswith(" S") or
            line.startswith(" L") or
            line.startswith(" M"))


def analyzeTrace(argv):
    """
    determines the number of code and data pages from a valgrind lackey trace

    """
    # read marker from file
    marker_start, marker_end = readMarkerFile(argv)
    #print marker_start, marker_end

    # initialize counter variables
    codepages, datapages = [], []
    outsideCount = 0
    insideCount = 0

    # open file and process it line by line
    with open(argv[1], 'r') as f:
        reading = False
        for line in f:
            #ignore comment lines at beginning
            if not line.startswith("=="):
                # only count when between mem markers
                if not reading and line.split(" ")[-1].startswith(marker_start):
                    reading = True
                elif line.split(" ")[-1].startswith(marker_end):
                    reading = False
                
                if reading:
                    address = line.split(" ")[-1]# find the block with the addr
                    pagenumber = address.split(",")[0][0:-3]# cut out the offset
                    print address, pagenumber

                    if isDataPage(line):
                        if(pagenumber not in datapages):
                            datapages.append(pagenumber)
                        insideCount += 1

                    elif pagenumber not in codepages:
                        codepages.append(pagenumber)
                
                elif isDataPage(line):
                    outsideCount += 1

    #print codepages, datapages
    return len(codepages), len(datapages), outsideCount, insideCount


if __name__ == "__main__":
    if len(sys.argv) != 2 and len(sys.argv) != 3:
        print( "analyze.py: no tracefile specified.\n"
               "usage is \"analyze.py <tracefile> <markerfile>\"\n"
               "where <markerfile> defaults to \"./marker\" if not provided"
             )
    else:
        print analyzeTrace(sys.argv)
