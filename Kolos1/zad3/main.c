#define _POSIX_C_SOURCE 200809L        /* execvp, dup2, waitpid … */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    if (argc == 2) {
        /* ---------------------------------------------------------------- */
        char *filename1 = argv[1];

        int fd[2];
        if (pipe(fd) == -1) { perror("pipe"); return 1; }

        pid_t pid = fork();
        if (pid == -1) { perror("fork"); return 1; }

        if (pid == 0) {                              /* ===== POTOMEK ===== */
            /* zamykamy nieużywaną stronę do zapisu */
            close(fd[1]);             /* (potok tu w praktyce nie jest użyty) */
            /* uruchamiamy: sort --reverse filename1 */
            execlp("sort", "sort", "--reverse", filename1, (char *)NULL);

            /* jeśli execlp wróci – mamy błąd */
            perror("execlp");
            _exit(3);
        }
        else {                                       /* ===== RODZIC  ===== */
            close(fd[0]);                            /* nie czytamy z potoku */
            waitpid(pid, NULL, 0);                   /* czekamy na dziecko   */
        }
    }
    /* -------------------------------------------------------------------- */
    else if (argc == 3) {
        char *filename1 = argv[1];
        char *filename2 = argv[2];

        /* otwórz/utwórz filename2 z rwxr--r--  (0644 + execute dla właśc.)   */
        int filefd = open(filename2,
                          O_WRONLY | O_CREAT | O_TRUNC,
                          S_IRUSR | S_IWUSR | S_IXUSR |   /* owner rwx */
                          S_IRGRP |                       /* group  r  */
                          S_IROTH);                       /* others r  */
        if (filefd == -1) { perror("open"); return 2; }

        int fd[2];
        if (pipe(fd) == -1) { perror("pipe"); return 1; }

        pid_t pid = fork();
        if (pid == -1) { perror("fork"); return 1; }

        if (pid == 0) {                                  /* ===== POTOMEK ===== */
            close(fd[1]);                                /* zamykamy zapis      */

            /* przekieruj stdout → plik */
            if (dup2(filefd, STDOUT_FILENO) == -1) {
                perror("dup2");
                _exit(3);
            }
            close(filefd);                               /* już niepotrzebny    */

            /* sort filename1 */
            execlp("sort", "sort", filename1, (char *)NULL);

            /* jeżeli wrócimy – błąd */
            perror("execlp");
            _exit(3);
        }
        else {                                           /* ===== RODZIC  ===== */
            close(fd[0]);            /* nie czytamy - niepotrzebne            */
            close(filefd);           /* rodzic nie używa pliku                */
            waitpid(pid, NULL, 0);   /* czekamy na potomka                    */
        }
    }
    else {
        printf("Wrong number of args!\n");
    }

    return 0;
}