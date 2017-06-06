#!/bin/bash
make clean && make
rm -f *_log.txt
./othello -s 12345
cat server_log.txt
