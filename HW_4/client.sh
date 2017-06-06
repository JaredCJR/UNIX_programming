#!/bin/bash
./othello -c localhost:$1
cat client_log.txt
