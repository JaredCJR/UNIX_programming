
CC	= gcc
CFLAGS	= -Wall -g -I/usr/include/ncurses
LDFLAGS	= -lncurses -pthread
PROGS	= sample

all: $(PROGS)

sample: sample.o othello.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f *.o *~ $(PROGS)

