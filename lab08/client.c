#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

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

void sem_op(int sem_num, int op) {
    struct sembuf sb = { sem_num, op, 0 };
    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

void generate_task(char *task) {
    for (int i = 0; i < TASK_SIZE; ++i)
        task[i] = 'a' + rand() % 26;
    task[TASK_SIZE] = '\0';
}

int main(void) {
    srand(time(NULL) ^ getpid());

    shmid = shmget(SHM_KEY, sizeof(Queue), 0666);
    if (shmid == -1) { perror("shmget"); exit(1); }
    queue = shmat(shmid, NULL, 0);
    if (queue == (void *)-1) { perror("shmat"); exit(1); }

    semid = semget(SEM_KEY, 3, 0666);
    if (semid == -1) { perror("semget"); exit(1); }

    while (1) {
        char task[TASK_SIZE + 1];
        generate_task(task);

        sem_op(SEM_EMPTY, -1);
        sem_op(SEM_MUTEX, -1);

        strncpy(queue->tasks[queue->tail], task, TASK_SIZE + 1);
        queue->tail = (queue->tail + 1) % MAX_QUEUE;
        queue->count++;

        sem_op(SEM_MUTEX, 1);
        sem_op(SEM_FULL,  1);

        printf("Wysłano zadanie: %s\n", task);
        fflush(stdout);

        sleep(1 + rand() % 5);
    }

    return 0;
}