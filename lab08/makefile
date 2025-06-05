CC      = gcc
CFLAGS  = -Wall -Wextra -std=c17 -O2

# targety
PRINTER = printer
CLIENT  = client

.PHONY: all clean

all: $(PRINTER) $(CLIENT)
	@echo "Zbudowano: $(PRINTER), $(CLIENT)"

$(PRINTER): printer.c
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT): client.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(PRINTER) $(CLIENT)
