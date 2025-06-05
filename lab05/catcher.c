#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

volatile sig_atomic_t mode_change_requests = 0;
volatile sig_atomic_t current_mode = 1;
volatile sig_atomic_t sender_pid = 0;
volatile sig_atomic_t should_respond = 0;

void sigint_handler(int sig) {
    (void)sig;
    static const char msg[] = "\nCTRL+C pressed.\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

void sigusr1_handler(int sig, siginfo_t *info, void *ucontext) {
    (void)sig;
    (void)ucontext;

    sender_pid = info->si_pid;
    int mode = info->si_value.sival_int;
    mode_change_requests++;
    current_mode = mode;
    should_respond = 1;
}

int main() {
    printf("Catcher PID: %d\n", getpid());

    struct sigaction sa;
    sa.sa_sigaction = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);

    struct sigaction sa_int;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sa_int.sa_handler = sigint_handler;

    int counter = 1;

    while (1) {
        if (current_mode == 2) {
            printf("Counter: %d\n", counter++);
            sleep(1);
        } else {
            counter = 1;
            pause();
        }

        if (should_respond) {
            switch (current_mode) {
                case 1:
                    printf("Received %d mode change requests\n", mode_change_requests);
                    break;
                case 3:
                    signal(SIGINT, SIG_IGN);
                    break;
                case 4:
                    sigaction(SIGINT, &sa_int, NULL);
                    break;
                case 5:
                    printf("Shutting down catcher...\n");
                    break;
            }

            union sigval val;
            val.sival_int = 0;
            sigqueue(sender_pid, SIGUSR1, val);
            should_respond = 0;

            if (current_mode == 5) {
                exit(0);
            }
        }
    }

    return 0;
}
