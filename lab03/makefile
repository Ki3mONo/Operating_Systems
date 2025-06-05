CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = flipper

all: $(TARGET)

$(TARGET): flipper.o
	$(CC) $(CFLAGS) -o $(TARGET) flipper.o

flipper.o: flipper.c
	$(CC) $(CFLAGS) -c flipper.c

clean:
	rm -f *.o $(TARGET)

.PHONY: all clean