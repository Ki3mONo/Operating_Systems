CC=gcc
CFLAGS=-Wall -fPIC -std=c17

all: static shared dynamic

static: collatz.o
	ar rcs libcollatz.a collatz.o

shared: collatz.o
	$(CC) $(CFLAGS) -shared -o libcollatz.so collatz.o

collatz.o: collatz.c
	$(CC) $(CFLAGS) -c collatz.c -o collatz.o

dynamic: client_dynamic.o
	$(CC) $(CFLAGS) client_dynamic.o -o collatz_dynamic_client -L. -lcollatz -ldl -Wl,-rpath=./

client_dynamic.o: client_dynamic.c
	$(CC) $(CFLAGS) -c client_dynamic.c -o client_dynamic.o

clean:
	rm -f *.o *.a *.so collatz_dynamic_client
