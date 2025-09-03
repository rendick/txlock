CC=gcc
CFLAGS=-g -Wall -Wextra -fsanitize=signed-integer-overflow
CLIBS=-lX11 -lcrypt

build:
	$(CC) $(CFLAGS) $(CLIBS) txlock.c -o txlock
