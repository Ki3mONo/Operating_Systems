CC      = gcc
CFLAGS  = -Wall -Wextra -std=c17 -O2
SERVER  = server
CLIENT  = client

all: $(SERVER) $(CLIENT)
	@echo "Zbudowano: $(SERVER), $(CLIENT)"

$(SERVER): server.c
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT): client.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all clean

clean:
	rm -f $(SERVER) $(CLIENT)
