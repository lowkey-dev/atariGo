CC = gcc
CFLAGS = -std=c99 -Wall -pedantic

default: nogo

nogo: game.o board.o move.o liberties.o
	$(CC) $(CFLAGS) -o nogo game.o board.o move.o liberties.o

game.o: game.c move.h board.h liberties.h
	$(CC) $(CFLAGS) -c game.c

move.o: move.c move.h board.h
	$(CC) $(CFLAGS) -c move.c

board.o: board.c board.h
	$(CC) $(CFLAGS) -c board.c

liberties.o: liberties.c liberties.h
	$(CC) $(CFLAGS) -c liberties.c