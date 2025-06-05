#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

static double f(double x) { return 4.0 / (x * x + 1.0); }

/* Zwraca czas w sekundach z dokładnością do nanosekund */
static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <szerokosc_h> <n>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const double h = atof(argv[1]);
    const int n_max = atoi(argv[2]);
    if (h <= 0.0 || n_max <= 0) {
        fprintf(stderr, "Błędne parametry.\n");
        return EXIT_FAILURE;
    }

    const long long Ntot = (long long)(1.0 / h);   /* Ile prostokątów w sumie */
    for (int k = 1; k <= n_max; ++k) {
        /*------ przygotowanie potoków ---------------*/
        int (*pipes)[2] = malloc(sizeof(int[2]) * k);
        if (!pipes) { perror("malloc"); exit(EXIT_FAILURE); }
        for (int i = 0; i < k; ++i)
            if (pipe(pipes[i]) == -1) { perror("pipe"); exit(EXIT_FAILURE); }

        double t0 = now_sec();

        /*------ tworzenie potomków ------------------*/
        for (int i = 0; i < k; ++i) {
            pid_t pid = fork();
            if (pid == -1) { perror("fork"); exit(EXIT_FAILURE); }
            if (pid == 0) {                /* ====== potomek ====== */
                /* zamykamy nieużywane końce */
                close(pipes[i][0]);        /* czytanie niepotrzebne */
                for (int j = 0; j < k; ++j)
                    if (j != i) { close(pipes[j][0]); close(pipes[j][1]); }

                /* fragment przedziału dla procesu i */
                long long start =  i    * Ntot / k;
                long long end   = (i+1) * Ntot / k;

                double partial = 0.0;
                for (long long n = start; n < end; ++n)
                    partial += f((n + 0.5) * h) * h;

                if (write(pipes[i][1], &partial, sizeof(partial)) != sizeof(partial))
                    perror("write");

                close(pipes[i][1]);
                _exit(0);                  /* ważne: nie wykonywać kodu rodzica */
            }
            /* ====== rodzic: kontynuuje tworzenie kolejnych dzieci ====== */
            close(pipes[i][1]);            /* nie piszemy */
        }

        /*------ rodzic: zbiera wyniki ---------------*/
        double result = 0.0, partial;
        for (int i = 0; i < k; ++i) {
            if (read(pipes[i][0], &partial, sizeof(partial)) == sizeof(partial))
                result += partial;
            close(pipes[i][0]);
        }

        /* czekamy na zakończenie wszystkich potomków */
        while (wait(NULL) > 0)
            ;

        double t1 = now_sec();
        printf("k=%d\twynik=%.12f\tczas=%.3f s\n", k, result, t1 - t0);
        free(pipes);
    }
    return 0;
}
