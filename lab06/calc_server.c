#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define IN_FIFO  "/tmp/int_in.fifo"
#define OUT_FIFO "/tmp/int_out.fifo"
static double f(double x) { return 4.0 / (x * x + 1.0); }

int main(void)
{
    /* Potoki mogą istnieć z poprzedniego uruchomienia */
    mkfifo(IN_FIFO, 0666);
    mkfifo(OUT_FIFO, 0666);

    int in_fd  = open(IN_FIFO,  O_RDONLY);
    if (in_fd < 0) { perror("open in"); exit(EXIT_FAILURE); }
    int out_fd = open(OUT_FIFO, O_WRONLY);
    if (out_fd < 0) { perror("open out"); exit(EXIT_FAILURE); }

    double a, b;
    if (read(in_fd, &a, sizeof(a)) != sizeof(a) ||
        read(in_fd, &b, sizeof(b)) != sizeof(b)) {
        fprintf(stderr, "Nie udało się odczytać przedziału.\n");
        return EXIT_FAILURE;
    }

    const double h = 1e-7;
    long long N = (long long)((b - a) / h);
    double sum = 0.0;
    for (long long i = 0; i < N; ++i)
        sum += f(a + (i + 0.5) * h) * h;

    if (write(out_fd, &sum, sizeof(sum)) != sizeof(sum))
        perror("write");

    close(in_fd);
    close(out_fd);
    return 0;
}
