CC      = gcc
CFLAGS  = -Wall -Wextra -std=c17 -O2 -pthread

# target
INTEGRAL = integral

.PHONY: all clean

all: $(INTEGRAL)
	@echo "Zbudowano: $(INTEGRAL)"

$(INTEGRAL): integral.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(INTEGRAL)
