#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_KEY 0x1234
#define MAX_MSG_LEN 256

#define MSG_INIT 1
#define MSG_CHAT 2

typedef struct {
    long mtype;
    pid_t pid;
    key_t client_key;
    char text[MAX_MSG_LEN];
} init_msg;

typedef struct {
    long mtype;
    int sender_id;
    char text[MAX_MSG_LEN];
} chat_msg;

int client_qid;

void handle_exit(int) {
    msgctl(client_qid, IPC_RMID, NULL);
    printf("\nZamknięto klienta.\n");
    exit(0);
}

void receive_messages() {
    while (1) {
        chat_msg msg;
        if (msgrcv(client_qid, &msg, sizeof(msg) - sizeof(long), 0, 0) > 0) {
            if (msg.mtype == MSG_CHAT) {
                printf("[Klient %d]: %s\n", msg.sender_id, msg.text);
            }
        }
    }
}

int main() {
    signal(SIGINT, handle_exit);

    key_t client_key = ftok(".", getpid());
    client_qid = msgget(client_key, IPC_CREAT | 0666);
    if (client_qid == -1) {
        perror("msgget (client)");
        exit(1);
    }

    int server_qid = msgget(SERVER_KEY, 0);
    if (server_qid == -1) {
        perror("msgget (server)");
        exit(1);
    }

    init_msg msg = { .mtype = MSG_INIT, .pid = getpid(), .client_key = client_key };
    msgsnd(server_qid, &msg, sizeof(init_msg) - sizeof(long), 0);

    chat_msg id_msg;
    msgrcv(client_qid, &id_msg, sizeof(chat_msg) - sizeof(long), MSG_INIT, 0);
    int my_id = atoi(id_msg.text);

    printf("Połączono z serwerem. Twój ID: %d\n", my_id);

    if (fork() == 0) {
        receive_messages();
        exit(0);
    }

    char buffer[MAX_MSG_LEN];
    while (fgets(buffer, MAX_MSG_LEN, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        init_msg chat = { .mtype = MSG_CHAT, .pid = my_id };
        snprintf(chat.text, MAX_MSG_LEN, "%s", buffer);
        chat.text[MAX_MSG_LEN - 1] = '\0';
        msgsnd(server_qid, &chat, sizeof(init_msg) - sizeof(long), 0);
    }

    return 0;
}
