#!/bin/sh

./diffviewer.py img/0-emptydisk.img img/1-onefile.img > d0-1
./diffviewer.py img/1-onefile.img img/2-deletedfile.img > d1-2
./diffviewer.py img/2-deletedfile.img img/3-onedirectory.img > d2-3
./diffviewer.py img/3-onedirectory.img img/4-hardlink.img > d3-4
./diffviewer.py img/4-hardlink.img img/5-deleteddirectory.img > d4-5

xxd -c 16 ./img/0-emptydisk.img > img/dump-0
xxd -c 16 ./img/1-onefile.img > img/dump-1
xxd -c 16 ./img/2-deletedfile.img > img/dump-2
xxd -c 16 ./img/3-onedirectory.img > img/dump-3
xxd -c 16 ./img/4-hardlink.img > img/dump-4
xxd -c 16 ./img/5-deleteddirectory.img > img/dump-5
