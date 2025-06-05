CC = gcc
CFLAGS = -Wall -Wextra -std=c17
TARGET1 = zadanie1
TARGET2 = zadanie2

all: $(TARGET1) $(TARGET2)

$(TARGET1): zadanie1.o
	$(CC) $(CFLAGS) -o $(TARGET1) zadanie1.o

$(TARGET2): zadanie2.o
	$(CC) $(CFLAGS) -o $(TARGET2) zadanie2.o

zadanie1.o: zadanie1.c
	$(CC) $(CFLAGS) -c zadanie1.c

zadanie2.o: zadanie2.c
	$(CC) $(CFLAGS) -c zadanie2.c

clean:
	rm -f *.o $(TARGET1) $(TARGET2)

.PHONY: all clean
