
CC	= gcc
CFLAGS	= -Wall -g -I/usr/include/ncurses -O0
LDFLAGS	= -lncurses -pthread
PROGS	= othello

all: $(PROGS)

gdb_client: $(PROGS)
	gdbtui --args ./$(PROGS) -c localhost:12345

$(PROGS): othello.o othello_lib.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f *.o *~ $(PROGS) *_log.txt

