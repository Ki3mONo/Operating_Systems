// ./integral 1e-10 20 około 6 sekund
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int    idx;
    long   start;
    long   count;
    double dx;
    double *results;
} thread_data_t;

void *worker(void *arg) {
    thread_data_t *d = (thread_data_t*)arg;
    double sum = 0.0, x;
    long i, end = d->start + d->count;
    for (i = d->start; i < end; ++i) {
        x = i * d->dx;
        sum += (4.0 / (x*x + 1.0)) * d->dx;
    }
    d->results[d->idx] = sum;
    return NULL;
}

static double elapsed_sec(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec)
        + (end->tv_nsec - start->tv_nsec) * 1e-9;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <dx> <liczba_wątków>\n", argv[0]);
        return EXIT_FAILURE;
    }
    double dx = atof(argv[1]);
    int k = atoi(argv[2]);
    if (dx <= 0.0 || k <= 0) {
        fprintf(stderr, "Błędne parametry: dx>0, k>0\n");
        return EXIT_FAILURE;
    }

    long N = (long)(1.0 / dx);
    if (N < k) k = (int)N;

    pthread_t    *threads = malloc(k * sizeof(*threads));
    thread_data_t *data    = malloc(k * sizeof(*data));
    double       *results  = calloc(k, sizeof(*results));
    if (!threads || !data || !results) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    // podział pracy
    long base = N / k, rem = N % k, offset = 0;
    for (int i = 0; i < k; ++i) {
        long cnt = base + (i < rem ? 1 : 0);
        data[i] = (thread_data_t){ .idx = i,
                                    .start = offset,
                                    .count = cnt,
                                    .dx = dx,
                                    .results = results };
        offset += cnt;
        pthread_create(&threads[i], NULL, worker, &data[i]);
    }
    for (int i = 0; i < k; ++i)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t_end);

    double integral = 0.0;
    for (int i = 0; i < k; ++i)
        integral += results[i];

    printf("Wartość całki ≈ %.15f (dx=%.3g, wątków=%d)\n",
            integral, dx, k);
    printf("Czas obliczeń: %.6f s\n", elapsed_sec(&t_start, &t_end));

    free(threads);
    free(data);
    free(results);
    return 0;
}
