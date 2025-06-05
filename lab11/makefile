CC      = gcc
CFLAGS  = -Wall -Wextra -std=c17 -O2

# targety
SERVER  = server
CLIENT  = client

.PHONY: all clean

all: $(SERVER) $(CLIENT)
	@echo "Zbudowano: $(SERVER), $(CLIENT)"

$(SERVER): server.c
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT): client.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(SERVER) $(CLIENT)