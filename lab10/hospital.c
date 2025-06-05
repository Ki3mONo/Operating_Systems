#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#define MEDS_CAPACITY 6
#define CONSULT_GROUP 3

// --- zmienne globalne i synchronizacja ---
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_doctor      = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond_patients    = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond_space       = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond_pharm_ack   = PTHREAD_COND_INITIALIZER;

static int waiting_count       = 0;             // ilu pacjentów w poczekalni
static int waiting_ids[CONSULT_GROUP];          // ich ID
static int meds_count          = MEDS_CAPACITY; // aktualna liczba leków
static int served_count        = 0;             // ilu pacjentów już obsłużono
static int total_patients;                      // z main
static int total_pharmacists;                   // z main
static int pharm_waiting_count = 0;             // ilu farmaceutów czeka

// pomocnicze: losowy przedział [min..max]
static int rand_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// wypisywanie z dokładnym czasem
static void log_msg(const char *fmt, ...) {
    char buf[64];
    time_t t = time(NULL);
    struct tm lt;
    localtime_r(&t, &lt);
    strftime(buf, sizeof(buf), "%H:%M:%S", &lt);

    printf("[%s] - ", buf);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    fflush(stdout);
}

// --- wątek lekarza ---
void *doctor_thread(void *arg) {
    (void)arg;
    pthread_mutex_lock(&mutex);
    while (served_count < total_patients) {
        // warunek przebudzenia:
        while (!((waiting_count >= CONSULT_GROUP && meds_count >= CONSULT_GROUP)
                 || (pharm_waiting_count > 0 && meds_count < CONSULT_GROUP)
                 || (served_count + waiting_count == total_patients && waiting_count > 0 && meds_count >= waiting_count))) {
            pthread_cond_wait(&cond_doctor, &mutex);
        }

        log_msg("Lekarz: budzę się");

        // 1) obsługa pełnej grupy pacjentów
        if (waiting_count >= CONSULT_GROUP && meds_count >= CONSULT_GROUP) {
            log_msg("Lekarz: konsultuję pacjentów %d, %d, %d",
                    waiting_ids[0], waiting_ids[1], waiting_ids[2]);
            meds_count -= CONSULT_GROUP;
            served_count += CONSULT_GROUP;
            waiting_count = 0;
            pthread_cond_broadcast(&cond_space);
            pthread_cond_broadcast(&cond_patients);

            pthread_mutex_unlock(&mutex);
            sleep(rand_range(2,4));
            pthread_mutex_lock(&mutex);
            log_msg("Lekarz: zasypiam");
        }
        // 2) obsługa ostatniej mniejszej grupy
        else if (served_count + waiting_count == total_patients && waiting_count > 0 && meds_count >= waiting_count) {
            // wypisujemy dokładnie oczekujących
            if (waiting_count == 1) {
                log_msg("Lekarz: konsultuję ostatniego pacjenta %d", waiting_ids[0]);
            } else if (waiting_count == 2) {
                log_msg("Lekarz: konsultuję ostatnich pacjentów %d i %d", waiting_ids[0], waiting_ids[1]);
            }
            meds_count -= waiting_count;
            served_count += waiting_count;
            waiting_count = 0;
            pthread_cond_broadcast(&cond_space);
            pthread_cond_broadcast(&cond_patients);

            pthread_mutex_unlock(&mutex);
            sleep(rand_range(2,4));
            pthread_mutex_lock(&mutex);
            log_msg("Lekarz: zasypiam");
        }
        // 3) odbiór dostawy farmaceuty
        else if (pharm_waiting_count > 0 && meds_count < CONSULT_GROUP) {
            log_msg("Lekarz: przyjmuję dostawę leków");
            pthread_cond_signal(&cond_pharm_ack);

            pthread_mutex_unlock(&mutex);
            sleep(rand_range(1,3));
            pthread_mutex_lock(&mutex);
            log_msg("Lekarz: zasypiam");
        }
    }
    // kończymy: budzimy wszystkich (żeby nie wisieli)
    pthread_cond_broadcast(&cond_patients);
    pthread_cond_broadcast(&cond_space);
    pthread_cond_broadcast(&cond_pharm_ack);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

// --- wątek pacjenta ---
void *patient_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);

    while (1) {
        int t = rand_range(2,5);
        log_msg("Pacjent(%d): idę do szpitala, będę za %d s", id, t);
        sleep(t);

        pthread_mutex_lock(&mutex);
        if (served_count >= total_patients) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        if (waiting_count >= CONSULT_GROUP) {
            int t2 = rand_range(2,5);
            log_msg("Pacjent(%d): za dużo pacjentów, wracam później za %d s", id, t2);
            pthread_mutex_unlock(&mutex);
            sleep(t2);
            continue;
        }
        waiting_ids[waiting_count] = id;
        waiting_count++;
        log_msg("Pacjent(%d): czeka %d pacjentów na lekarza", id, waiting_count);
        if (waiting_count == CONSULT_GROUP || (served_count + waiting_count == total_patients)) {
            log_msg("Pacjent(%d): budzę lekarza", id);
            pthread_cond_signal(&cond_doctor);
        }
        pthread_cond_wait(&cond_patients, &mutex);
        pthread_mutex_unlock(&mutex);

        log_msg("Pacjent(%d): kończę wizytę", id);
        break;
    }
    return NULL;
}

// --- wątek farmaceuty ---
void *pharmacist_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);

    while (1) {
        int t = rand_range(5,15);
        log_msg("Farmaceuta(%d): idę do szpitala, będę za %d s", id, t);
        sleep(t);

        pthread_mutex_lock(&mutex);
        if (served_count >= total_patients) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        while (meds_count == MEDS_CAPACITY && served_count < total_patients) {
            pharm_waiting_count++;
            log_msg("Farmaceuta(%d): czekam na opróżnienie apteczki", id);
            pthread_cond_wait(&cond_space, &mutex);
            pharm_waiting_count--;
        }
        if (served_count >= total_patients) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        if (meds_count < CONSULT_GROUP) {
            pharm_waiting_count++;
            log_msg("Farmaceuta(%d): budzę lekarza", id);
            pthread_cond_signal(&cond_doctor);
            pthread_cond_wait(&cond_pharm_ack, &mutex);
            pharm_waiting_count--;

            meds_count = MEDS_CAPACITY;
            log_msg("Farmaceuta(%d): dostarczam leki", id);
            pthread_cond_broadcast(&cond_space);
            log_msg("Farmaceuta(%d): zakończyłem dostawę", id);
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <#pacjentów> <#farmaceutów>\n", argv[0]);
        return EXIT_FAILURE;
    }
    total_patients    = atoi(argv[1]);
    total_pharmacists = atoi(argv[2]);
    if (total_patients <= 0 || total_pharmacists <= 0) {
        fprintf(stderr, "Liczba pacjentów i farmaceutów musi być >0\n");
        return EXIT_FAILURE;
    }
    srand(time(NULL));

    pthread_t doc;
    pthread_create(&doc, NULL, doctor_thread, NULL);

    pthread_t *patients = malloc(sizeof(pthread_t) * total_patients);
    for (int i = 0; i < total_patients; i++) {
        int *id = malloc(sizeof(int)); *id = i+1;
        pthread_create(&patients[i], NULL, patient_thread, id);
    }

    pthread_t *pharms = malloc(sizeof(pthread_t) * total_pharmacists);
    for (int i = 0; i < total_pharmacists; i++) {
        int *id = malloc(sizeof(int)); *id = i+1;
        pthread_create(&pharms[i], NULL, pharmacist_thread, id);
    }

    for (int i = 0; i < total_patients;   i++) pthread_join(patients[i], NULL);
    for (int i = 0; i < total_pharmacists;i++) pthread_join(pharms[i], NULL);
    pthread_join(doc, NULL);

    free(patients);
    free(pharms);
    return 0;
}
