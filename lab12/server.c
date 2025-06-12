#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 1024
#define PING_INTERVAL 5   // seconds
#define PING_TIMEOUT 10   // seconds

typedef struct {
    struct sockaddr_in addr;
    socklen_t addr_len;
    char id[32];
    time_t last_seen;
    int active;
} client_t;

client_t clients[MAX_CLIENTS];
int sockfd;

void cleanup_and_exit() {
    close(sockfd);
    exit(0);
}

void sigint_handler(int sig) {
    (void)sig;
    cleanup_and_exit();
}

int find_client_by_addr(struct sockaddr_in *a, socklen_t alen) {
    (void)alen;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active &&
            clients[i].addr.sin_port == a->sin_port &&
            clients[i].addr.sin_addr.s_addr == a->sin_addr.s_addr)
            return i;
    }
    return -1;
}

int find_client_by_id(const char *id) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].active && strcmp(clients[i].id, id) == 0)
            return i;
    return -1;
}

int add_client(struct sockaddr_in *a, socklen_t alen, const char *id) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].addr = *a;
            clients[i].addr_len = alen;
            strncpy(clients[i].id, id, sizeof(clients[i].id)-1);
            clients[i].id[sizeof(clients[i].id)-1] = '\0';
            clients[i].last_seen = time(NULL);
            clients[i].active = 1;
            printf("Client %s registered %s:%d\n",
                   id, inet_ntoa(a->sin_addr), ntohs(a->sin_port));
            return i;
        }
    }
    return -1;
}

void remove_client(int idx) {
    printf("Client %s disconnected\n", clients[idx].id);
    clients[idx].active = 0;
}

void send_to(struct sockaddr_in *a, socklen_t alen, const char *msg) {
    sendto(sockfd, msg, strlen(msg), 0,
           (struct sockaddr*)a, alen);
}

void broadcast(const char *msg, int exclude_idx) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && i != exclude_idx)
            send_to(&clients[i].addr, clients[i].addr_len, msg);
    }
}

void do_list(int idx) {
    char buf[BUF_SIZE] = "ACTIVE:";
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            strcat(buf, " ");
            strcat(buf, clients[i].id);
        }
    }
    strcat(buf, "\n");
    send_to(&clients[idx].addr, clients[idx].addr_len, buf);
}

char* current_time_str() {
    static char ts[64];
    time_t t = time(NULL);
    struct tm *p = localtime(&t);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", p);
    return ts;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    signal(SIGINT, sigint_handler);
    memset(clients, 0, sizeof(clients));

    int port = atoi(argv[1]);
    struct sockaddr_in srv;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = INADDR_ANY;
    srv.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&srv, sizeof(srv)) < 0) {
        perror("bind"); exit(1);
    }
    printf("UDP server listening on port %d\n", port);

    fd_set readfds;
    time_t last_ping = time(NULL);
    char buf[BUF_SIZE];

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
        select(sockfd+1, &readfds, NULL, NULL, &tv);

        if (FD_ISSET(sockfd, &readfds)) {
            struct sockaddr_in cli;
            socklen_t clen = sizeof(cli);
            int r = recvfrom(sockfd, buf, sizeof(buf)-1, 0,
                             (struct sockaddr*)&cli, &clen);
            if (r < 0) continue;
            buf[r] = '\0';
            int idx = find_client_by_addr(&cli, clen);
            char *cmd = strtok(buf, " \r\n");
            if (!cmd) continue;

            if (strcmp(cmd, "REG") == 0) {
                char *id = strtok(NULL, "\r\n");
                if (idx < 0) {
                    if (add_client(&cli, clen, id) < 0)
                        send_to(&cli, clen, "FULL\n");
                }
            } else if (idx >= 0) {
                clients[idx].last_seen = time(NULL);
                if (strcmp(cmd, "LIST") == 0) {
                    do_list(idx);
                } else if (strcmp(cmd, "2ALL") == 0) {
                    char *msg = strtok(NULL, "");
                    char out[BUF_SIZE];
                    snprintf(out, sizeof(out), "MSG %s %s %s\n",
                             clients[idx].id, current_time_str(), msg);
                    broadcast(out, idx);
                } else if (strcmp(cmd, "2ONE") == 0) {
                    char *dst = strtok(NULL, " \r\n");
                    char *msg = strtok(NULL, "");
                    int j = find_client_by_id(dst);
                    if (j >= 0) {
                        char out[BUF_SIZE];
                        snprintf(out, sizeof(out), "MSG %s %s %s\n",
                                 clients[idx].id, current_time_str(), msg);
                        send_to(&clients[j].addr, clients[j].addr_len, out);
                    }
                } else if (strcmp(cmd, "STOP") == 0) {
                    remove_client(idx);
                } else if (strcmp(cmd, "ALIVE") == 0) {

                }
            }
        }

        time_t now = time(NULL);
        if (now - last_ping >= PING_INTERVAL) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active)
                    send_to(&clients[i].addr, clients[i].addr_len, "PING\n");
            }
            last_ping = now;
        }

        /* remove timed-out clients */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active && now - clients[i].last_seen >= PING_TIMEOUT)
                remove_client(i);
        }
    }

    return 0;
}
