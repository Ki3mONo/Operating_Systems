#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int global = 0;

void zadanie2(const char *dir) {
    int local = 0;

    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        printf("child process\n");
        global++;
        local++;
        printf("child pid = %d, parent pid = %d\n", getpid(), getppid());
        printf("child's local = %d, child's global = %d\n", local, global);
        execl("/bin/ls", "ls", dir, NULL);
        perror("execl failed");
        exit(1);
    } else {
        int status;
        wait(&status);
        printf("parent process\n");
        printf("parent pid = %d, child pid = %d\n", getpid(), pid);
        printf("Child exit code: %d\n", WEXITSTATUS(status));
        printf("Parent's local = %d, parent's global = %d\n", local, global);
    }
}

int main(int argc, char *argv[]) {

    printf("Program name: %s\n", argv[0]);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    zadanie2(argv[1]);

    return 0;
}
