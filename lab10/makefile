CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 -pthread

HOSPITAL = hospital

.PHONY: all clean

all: $(HOSPITAL)
	@echo "Zbudowano: $(HOSPITAL)"

$(HOSPITAL): hospital.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(HOSPITAL)
