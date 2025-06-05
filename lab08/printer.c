#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

#define MAX_QUEUE   10
#define TASK_SIZE   10
#define SHM_KEY     0x1111
#define SEM_KEY     0x2222

typedef struct {
    char tasks[MAX_QUEUE][TASK_SIZE + 1];
    int head;
    int tail;
    int count;
} Queue;

enum {
    SEM_MUTEX = 0,
    SEM_FULL  = 1,
    SEM_EMPTY = 2
};

static int shmid = -1, semid = -1;
static Queue *queue = NULL;

void cleanup(int sig) {
    (void)sig;
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    printf("\nZasoby IPC usunięte.\n");
    exit(0);
}

void sem_op(int sem_num, int op) {
    struct sembuf sb = { sem_num, op, 0 };
    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

void print_task(const char *text, int printer_id) {
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "[Drukarka %d] %.*s\n",
                       printer_id, TASK_SIZE, text);
    if (write(STDOUT_FILENO, buf, len) == -1) {
        perror("write");
        exit(1);
    }
}

void printer_loop(int printer_id) {
    while (1) {
        sem_op(SEM_FULL,  -1);
        sem_op(SEM_MUTEX, -1);

        char task[TASK_SIZE + 1];
        strncpy(task, queue->tasks[queue->head], TASK_SIZE);
        task[TASK_SIZE] = '\0';
        queue->head = (queue->head + 1) % MAX_QUEUE;
        queue->count--;

        sem_op(SEM_MUTEX, 1);
        sem_op(SEM_EMPTY, 1);

        print_task(task, printer_id);
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Użycie: %s <liczba_drukarek>\n", argv[0]);
        return 1;
    }
    int printer_count = atoi(argv[1]);
    if (printer_count <= 0) {
        fprintf(stderr, "Liczba drukarek musi być > 0\n");
        return 1;
    }

    signal(SIGINT,  cleanup);
    signal(SIGTERM, cleanup);

    shmid = shmget(SHM_KEY, sizeof(Queue), IPC_CREAT | 0666);
    if (shmid == -1) { perror("shmget"); exit(1); }

    queue = shmat(shmid, NULL, 0);
    if (queue == (void *)-1) { perror("shmat"); exit(1); }

    queue->head = queue->tail = queue->count = 0;

    semid = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (semid == -1) { perror("semget"); exit(1); }
    semctl(semid, SEM_MUTEX, SETVAL, 1);
    semctl(semid, SEM_FULL,  SETVAL, 0);
    semctl(semid, SEM_EMPTY, SETVAL, MAX_QUEUE);

    printf("Serwer uruchomiony z %d drukarkami.\n", printer_count);
    fflush(stdout);

    for (int i = 0; i < printer_count; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            printer_loop(i + 1);
        }
    }

    pause();
    return 0;
}