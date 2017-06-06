#!/bin/bash
make clean && make
rm *_log.txt
./othello -s 12345
cat server_log.txt
