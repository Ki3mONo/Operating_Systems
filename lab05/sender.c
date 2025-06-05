#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t response_received = 0;

void sigusr1_handler(int sig) {
    (void)sig;
    response_received = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <catcher PID> <mode (1-5)>\n", argv[0]);
        return 1;
    }

    pid_t catcher_pid = atoi(argv[1]);
    int mode = atoi(argv[2]);

    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    union sigval val;
    val.sival_int = mode;
    sigqueue(catcher_pid, SIGUSR1, val);

    sigset_t mask;
    sigemptyset(&mask);
    while (!response_received) {
        sigsuspend(&mask);
    }

    printf("Confirmation received from catcher.\n");
    return 0;
}
