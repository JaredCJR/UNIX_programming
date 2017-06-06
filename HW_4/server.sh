#!/bin/bash
make clean && make
rm -f *_log.txt
./othello -s $1
cat server_log.txt
