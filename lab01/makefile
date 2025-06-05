CC = gcc
CFLAGS = -std=c17 -Wall
sources = countdown

.PHONY: all clean

all: countdown

countdown: countdown.c
	$(CC) countdown.c $(CFLAGS) -o countdown

clean:
	rm -f $(sources)

