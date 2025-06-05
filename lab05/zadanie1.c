#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void sigusr1_handler(int signal_number) {
    printf("Received SIGUSR1 with number: %d\n", signal_number);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <none|ignore|handler|mask>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], "ignore")) {
        signal(SIGUSR1, SIG_IGN);
        raise(SIGUSR1);
    }
    else if (!strcmp(argv[1], "handler")) {
        signal(SIGUSR1, sigusr1_handler);
        raise(SIGUSR1);
    }
    else if (!strcmp(argv[1], "mask")) {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        sigprocmask(SIG_BLOCK, &set, NULL);
        raise(SIGUSR1);

        sigset_t pending;
        sigpending(&pending);

        printf("Is SIGUSR1 pending? : %d\n", sigismember(&pending, SIGUSR1));
    }
    else {
        signal(SIGUSR1, SIG_DFL);
        raise(SIGUSR1);
    }

    return 0;
}
