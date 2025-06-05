CC = gcc
CFLAGS = -Wall -Wextra -std=c17
TARGET1 = zadanie1
TARGET2 = zadanie2
OBJECTS2 = catcher.o sender.o

all: $(TARGET1) $(TARGET2)

$(TARGET1): zadanie1.o
	$(CC) $(CFLAGS) -o $(TARGET1) zadanie1.o

$(TARGET2): catcher sender
	@echo "Zadanie2 zbudowane: catcher i sender"

catcher: catcher.o
	$(CC) $(CFLAGS) -o catcher catcher.o

sender: sender.o
	$(CC) $(CFLAGS) -o sender sender.o

catcher.o: catcher.c
	$(CC) $(CFLAGS) -c catcher.c

sender.o: sender.c
	$(CC) $(CFLAGS) -c sender.c

zadanie1.o: zadanie1.c
	$(CC) $(CFLAGS) -c zadanie1.c

clean:
	rm -f *.o $(TARGET1) catcher sender

.PHONY: all clean
