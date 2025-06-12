#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAX 20

int main(void)
{
	// Korzystając z funkcji POSIX:
	// ____________________________
	// Utwórz segment pamięci współdzielonej o prawach dostępu 600, rozmiarze MAX
	// Jeśli segment już istnieje, to zwróć błąd i zakończ program z wartością 1
	// Jeśli utworzenie segmentu pamięci się nie powiedzie, to też zwróć błąd i zakończ program z wartością 1

	// Przyłącz segment pamięci współdzielonej do procesu, obsłuż błędy i zwróć 1 (w przypadku błędu)
	// Podłączając pamięć ustaw prawa dostępu do mapowanej pamięci: odczyt, zapis
	// Specygikacja użycia segmentu -  współdzielony przez procesy
	const char *shm_name = "/my_shm";
    int fd;
    int *buf;
    size_t length = MAX * sizeof(int);

    fd = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "Segment '%s' już istnieje\n", shm_name);
        } else {
            perror("shm_open");
        }
        exit(1);
    }

    if (ftruncate(fd, length) == -1) {
        perror("ftruncate");
        shm_unlink(shm_name);
        exit(1);
    }

    buf = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buf == MAP_FAILED) {
        perror("mmap");
        shm_unlink(shm_name);
        exit(1);
    }

    close(fd);

	int i;
	for (i = 0; i < MAX; i++)
	{
		buf[i] = i * i;
		printf("Wartość: %d \n", buf[i]);
	}
	printf("Memory written\n");
	// Odłącz i  usuń segment pamięci współdzielonej

	if (munmap(buf, length) == -1) {
        perror("munmap");
    }
    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
        exit(1);
    }

	return 0;
}
