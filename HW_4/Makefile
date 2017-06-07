
CC	= g++
CFLAGS	= -Wall -g -I/usr/include/ncurses -O0
LDFLAGS	= -lncurses -pthread
PROGS	= othello

P ?= 12345

all: $(PROGS)

gdb_client: $(PROGS)
	gdbtui --args ./$(PROGS) -c localhost:$(P)
gdb_server: $(PROGS)
	gdbtui --args ./$(PROGS) -s $(P)

$(PROGS): othello.o othello_lib.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f *.o *~ $(PROGS) *_log.txt

