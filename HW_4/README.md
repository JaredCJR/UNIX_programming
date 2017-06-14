0556054   張家榮   jaredcjr.tw@gmail.com
==============================================

* All the features requested by the homework are done.

How to build and run:
===================================
```
cd /path/to/the/hw
make clean
make
./othello -s [port_number]

then open another terminal

cd /path/to/the/hw
./othello -c [ip or hostname]:[port_number]
```

Play once, and you will find all the requested functionalities are all perfectly done.
=========================================================================================
- Your program can act as either a server (player #1) or a client (player #2) by using the respective command arguments.
- A server has to wait for a client connection.
- A client can connect to the given server (IP address or host name).
- Once connected, display the game board. The game always starts from player #1 (server).
- Player can only put pieces (discs) on valid places (see game rule).
- Display correct number of pieces on the game board for the both players.
- Implement the rest of game logics.
- When there is no more moves, display a message to show the player wins or loses.
- Ensure the both two players have the same view of game board.
