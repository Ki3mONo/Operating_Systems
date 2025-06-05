#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define IN_FIFO  "/tmp/int_in.fifo"
#define OUT_FIFO "/tmp/int_out.fifo"

int main(void)
{
    double a, b;
    printf("Podaj początek i koniec przedziału (np. 0 1): ");
    if (scanf("%lf %lf", &a, &b) != 2 || a >= b) {
        fprintf(stderr, "Błąd danych.\n");
        return EXIT_FAILURE;
    }

    mkfifo(IN_FIFO, 0666);    /* w razie braku */
    mkfifo(OUT_FIFO, 0666);

    int in_fd  = open(IN_FIFO,  O_WRONLY);
    if (in_fd < 0) { perror("open in"); exit(EXIT_FAILURE); }
    int out_fd = open(OUT_FIFO, O_RDONLY);
    if (out_fd < 0) { perror("open out"); exit(EXIT_FAILURE); }

    if (write(in_fd, &a, sizeof(a)) != sizeof(a) ||
        write(in_fd, &b, sizeof(b)) != sizeof(b)) {
        perror("write a/b"); return EXIT_FAILURE;
    }

    double result;
    if (read(out_fd, &result, sizeof(result)) != sizeof(result)) {
        fprintf(stderr, "Nie udało się odczytać wyniku.\n");
        return EXIT_FAILURE;
    }

    printf("Wynik całki na [%g, %g] = %.12f\n", a, b, result);

    close(in_fd);
    close(out_fd);
    return 0;
}
