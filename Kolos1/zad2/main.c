#define _POSIX_C_SOURCE 200809L 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

static void sighandler(int signo, siginfo_t *info, void *ucontext)
{
    (void)ucontext;

    printf("[child] odebrano sygnał %d; przekazana wartość = %d\n",
           signo, info->si_value.sival_int);
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    /* --------- instalujemy obsługę SIGUSR1 (SA_SIGINFO) --------- */
    struct sigaction action;
    memset(&action, 0, sizeof action);
    action.sa_sigaction = &sighandler;
    action.sa_flags     = SA_SIGINFO;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    /* --------- tworzymy potomka --------- */
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }

    if (child == 0) {                    /* =========  POTOMEK  ========= */
        /* zablokuj wszystko oprócz SIGUSR1 */
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigprocmask(SIG_SETMASK, &mask, NULL);

        /* czekaj na sygnał */
        for (;;)
            pause();                     /* wybudza handler */
    }
    else {                               /* ==========  RODZIC  ========== */
        /* wysyłamy sygnał (argv[2]) wraz z wartością (argv[1]) */
        int sig_no = (int)strtol(argv[2], NULL, 10);   /* np. 10 == SIGUSR1 */
        union sigval val;
        val.sival_int = (int)strtol(argv[1], NULL, 10);

        if (sigqueue(child, sig_no, val) == -1) {
            perror("sigqueue");
        }

        /* mały delay, by potomek zdążył wypisać komunikat */
        sleep(1);
    }

    return 0;
}
