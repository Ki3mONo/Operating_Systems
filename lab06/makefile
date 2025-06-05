# ---------- Makefile for tasks 1 & 2 ----------
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c17 -O2

# ----------- targets ---------------------------
TARGET1 = zadanie1          # zadanie 1
SERVER  = calc_server                  # zadanie 2 – część 1
CLIENT  = calc_client                  # zadanie 2 – część 2

# „Zadanie 2” traktujemy jako zestaw dwóch plików wykonywalnych
TARGET2 = $(SERVER) $(CLIENT)

# Domyślna reguła
all: $(TARGET1) $(TARGET2)
	@echo "Zbudowano: $(TARGET1), $(SERVER), $(CLIENT)"

# ------------ zadanie 1 ------------------------
$(TARGET1): zadanie1.o
	$(CC) $(CFLAGS) -o $@ $^

zadanie1.o: zadanie1.c
	$(CC) $(CFLAGS) -c $<

# ------------ zadanie 2 ------------------------
$(SERVER): calc_server.o
	$(CC) $(CFLAGS) -o $@ $^

calc_server.o: calc_server.c
	$(CC) $(CFLAGS) -c $<

$(CLIENT): calc_client.o
	$(CC) $(CFLAGS) -o $@ $^

calc_client.o: calc_client.c
	$(CC) $(CFLAGS) -c $<

# ------------- housekeeping --------------------
clean:
	$(RM) *.o $(TARGET1) $(SERVER) $(CLIENT)

.PHONY: all clean
