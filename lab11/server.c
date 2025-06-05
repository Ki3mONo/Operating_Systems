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
#define PING_INTERVAL 5   // sekundy
#define PING_TIMEOUT 10   // sekundy

typedef struct {
    int fd;
    char id[32];
    time_t last_seen;
    int active;
} client_t;

client_t clients[MAX_CLIENTS];
int server_fd;

void cleanup_and_exit() {
    close(server_fd);
    for(int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].active)
            close(clients[i].fd);
    exit(0);
}

void sigint_handler(int sig) {
    (void)sig;
    cleanup_and_exit();
}

// Wyślij wiadomość do jednego klienta
void send_to_client(int idx, const char *msg) {
    send(clients[idx].fd, msg, strlen(msg), 0);
}

// Broadcast do wszystkich poza nadawcą (-1 = wszyscy)
void broadcast(const char *msg, int exclude_fd) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active && clients[i].fd != exclude_fd) {
            send_to_client(i, msg);
        }
    }
}

// Usuń klienta
void remove_client(int idx) {
    close(clients[idx].fd);
    clients[idx].active = 0;
    printf("Client %s disconnected\n", clients[idx].id);
}

int find_client_by_id(const char *id) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active && strcmp(clients[i].id, id) == 0)
            return i;
    }
    return -1;
}

int add_client(int fd, const char *id) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(!clients[i].active) {
            clients[i].fd = fd;
            strncpy(clients[i].id, id, sizeof(clients[i].id) - 1);
            clients[i].id[sizeof(clients[i].id) - 1] = '\0';
            clients[i].last_seen = time(NULL);
            clients[i].active = 1;
            printf("Client %s connected on fd %d\n", id, fd);
            return i;
        }
    }
    return -1;
}

void do_list(int idx) {
    char buf[BUF_SIZE];
    strcpy(buf, "ACTIVE:");
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active) {
            strcat(buf, " ");
            strcat(buf, clients[i].id);
        }
    }
    strcat(buf, "\n");
    send_to_client(idx, buf);
}

char* current_time_str() {
    static char ts[64];
    time_t t = time(NULL);
    struct tm *p = localtime(&t);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", p);
    return ts;
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    signal(SIGINT, sigint_handler);

    memset(clients, 0, sizeof(clients));

    int port = atoi(argv[1]);
    struct sockaddr_in addr;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) { perror("socket"); exit(1); }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); exit(1); }
    if(listen(server_fd, MAX_CLIENTS) < 0) { perror("listen"); exit(1); }

    printf("Server listening on port %d\n", port);

    fd_set readfds;
    int maxfd;
    time_t last_ping = time(NULL);

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        maxfd = server_fd;
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i].active) {
                FD_SET(clients[i].fd, &readfds);
                if(clients[i].fd > maxfd) maxfd = clients[i].fd;
            }
        }
        struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
        int activity = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        if(activity < 0 && errno != EINTR) { perror("select"); }

        // Nowe połączenie
        if(FD_ISSET(server_fd, &readfds)) {
            int newfd = accept(server_fd, NULL, NULL);
            if(newfd >= 0) {
                char buf[BUF_SIZE];
                int r = recv(newfd, buf, sizeof(buf) - 1, 0);
                if(r > 0) {
                    buf[r] = '\0';
                    if(strncmp(buf, "REG ", 4) == 0) {
                        char *id = buf + 4;
                        id[strcspn(id, "\r\n")] = '\0';
                        if(add_client(newfd, id) < 0) {
                            send(newfd, "FULL\n", 5, 0);
                            close(newfd);
                        }
                    } else {
                        close(newfd);
                    }
                } else close(newfd);
            }
        }

        // Odczyt od klientów
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i].active && FD_ISSET(clients[i].fd, &readfds)) {
                char buf[BUF_SIZE];
                int r = recv(clients[i].fd, buf, sizeof(buf) - 1, 0);
                if(r <= 0) {
                    remove_client(i);
                    continue;
                }
                buf[r] = '\0';
                char *cmd = strtok(buf, " \r\n");
                if(!cmd) continue;
                clients[i].last_seen = time(NULL);
                if(strcmp(cmd, "LIST") == 0) {
                    do_list(i);
                } else if(strcmp(cmd, "2ALL") == 0) {
                    char *msg = strtok(NULL, "");
                    char out[BUF_SIZE];
                    snprintf(out, sizeof(out), "MSG %s %s %s\n",
                             clients[i].id, current_time_str(), msg);
                    broadcast(out, clients[i].fd);
                } else if(strcmp(cmd, "2ONE") == 0) {
                    char *dst = strtok(NULL, " \r\n");
                    char *msg = strtok(NULL, "");
                    int j = find_client_by_id(dst);
                    if(j >= 0) {
                        char out[BUF_SIZE];
                        snprintf(out, sizeof(out), "MSG %s %s %s\n",
                                 clients[i].id, current_time_str(), msg);
                        send_to_client(j, out);
                    }
                } else if(strcmp(cmd, "STOP") == 0) {
                    remove_client(i);
                }
            }
        }

        // Ping i timeout klientów
        time_t now = time(NULL);
        if(now - last_ping >= PING_INTERVAL) {
            for(int i = 0; i < MAX_CLIENTS; i++) {
                if(clients[i].active) send_to_client(i, "PING\n");
            }
            last_ping = now;
        }
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i].active && now - clients[i].last_seen >= PING_TIMEOUT) {
                remove_client(i);
            }
        }
    }
    return 0;
}