CC=gcc
CFLAGS=-g -Wall -Wextra -fsanitize=signed-integer-overflow -0s
CLIBS=-lX11 -lcrypt

build:
	$(CC) $(BUILD_CFLAGS) $(CLIBS) txlock.c -o txlock
