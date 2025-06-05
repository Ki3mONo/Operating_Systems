#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void zadanie1(int num_children) {
    for (int i = 0; i < num_children; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            printf("Parent PID: %d, Child PID: %d\n", getppid(), getpid());
            exit(0);
        }
    }

    for (int i = 0; i < num_children; i++) {
        wait(NULL);
    }

    printf("%d\n", num_children);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_children>\n", argv[0]);
        return 1;
    }

    int num_children = atoi(argv[1]);
    zadanie1(num_children);

    return 0;
}
