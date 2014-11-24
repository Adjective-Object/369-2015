#!/bin/sh

./diffviewer.py img/0-emptydisk.img img/1-onefile.img > d0-1
./diffviewer.py img/1-onefile.img img/2-deletedfile.img > d1-2
./diffviewer.py img/3-onedirectory.img img/4-hardlink.img > d3-4
./diffviewer.py img/4-hardlink.img img/5-deleteddirectory.img > d4-5
