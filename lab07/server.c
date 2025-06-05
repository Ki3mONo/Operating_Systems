#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 256
#define SERVER_KEY 0x1234

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

typedef struct {
    int id;
    int qid;
    pid_t pid;
} client_info;

client_info clients[MAX_CLIENTS];
int client_count = 0;

void broadcast(int sender_id, const char *msg) {
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].id != sender_id) {
            chat_msg out_msg = { .mtype = MSG_CHAT, .sender_id = sender_id };
            strncpy(out_msg.text, msg, MAX_MSG_LEN - 1);
            out_msg.text[MAX_MSG_LEN - 1] = '\0';
            msgsnd(clients[i].qid, &out_msg, sizeof(chat_msg) - sizeof(long), 0);
        }
    }
}

int main() {
    int server_qid = msgget(SERVER_KEY, IPC_CREAT | 0666);
    if (server_qid == -1) {
        perror("msgget (server)");
        exit(1);
    }

    printf("Serwer uruchomiony. Oczekiwanie na klientów...\n");

    while (1) {
        init_msg msg;
        if (msgrcv(server_qid, &msg, sizeof(msg) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            continue;
        }

        if (msg.mtype == MSG_INIT) {
            if (client_count >= MAX_CLIENTS) {
                fprintf(stderr, "Maksymalna liczba klientów osiągnięta.\n");
                continue;
            }

            int client_qid = msgget(msg.client_key, 0);
            if (client_qid == -1) {
                perror("msgget (client)");
                continue;
            }

            int assigned_id = client_count + 1;
            clients[client_count++] = (client_info){.id = assigned_id, .qid = client_qid, .pid = msg.pid};

            chat_msg id_msg = { .mtype = MSG_INIT, .sender_id = 0 };
            snprintf(id_msg.text, MAX_MSG_LEN, "%d", assigned_id);
            msgsnd(client_qid, &id_msg, sizeof(chat_msg) - sizeof(long), 0);

            printf("Nowy klient dołączony (PID: %d, ID: %d)\n", msg.pid, assigned_id);
        } else if (msg.mtype == MSG_CHAT) {
            int sender_id = msg.pid;
            printf("Wiadomość od klienta %d: %s\n", sender_id, msg.text);
            broadcast(sender_id, msg.text);
        }
    }

    return 0;
}
