# ------- Kompilator i flagi -------
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c17 -O2

# ------- Pliki wynikowe ----------
SERVER  = server
CLIENT  = client

# ------- Reguła domyślna --------
all: $(SERVER) $(CLIENT)
	@echo "Zbudowano: $(SERVER), $(CLIENT)"

# ------- Kompilacja serwera -------
$(SERVER): server.o
	$(CC) $(CFLAGS) -o $@ $^

server.o: server.c
	$(CC) $(CFLAGS) -c $<

# ------- Kompilacja klienta -------
$(CLIENT): client.o
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c
	$(CC) $(CFLAGS) -c $<

# ----------- Sprzątanie ----------
clean:
	$(RM) *.o $(SERVER) $(CLIENT)

.PHONY: all clean
