NAME=txlock

CC=gcc
CFLAGS=-g -Wall -Wextra -fsanitize=signed-integer-overflow -Os
CLIBS=-lX11 -lcrypt

build:
	$(CC) $(BUILD_CFLAGS) $(CLIBS) $(NAME).c -o $(NAME)

clean:
	rm $(NAME)
