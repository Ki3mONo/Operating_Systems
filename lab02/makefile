CC=gcc
CFLAGS=-Wall -std=c17

.PHONY: all static shared dynamic clean

all: static shared dynamic

static:
	cd collatzlib && $(MAKE) static
	$(CC) $(CFLAGS) client.c -o client_static -Lcollatzlib -lcollatz

shared:
	cd collatzlib && $(MAKE) shared
	$(CC) $(CFLAGS) client.c -o client_shared -Lcollatzlib -lcollatz -ldl -Wl,-rpath=./collatzlib

dynamic:
	cd collatzlib && $(MAKE) shared
	$(CC) $(CFLAGS) -DDYNAMIC_LOAD client.c -o client_dynamic -Lcollatzlib -lcollatz -ldl -Wl,-rpath=./collatzlib

clean:
	rm -f client_static client_shared client_dynamic
	cd collatzlib && $(MAKE) clean
