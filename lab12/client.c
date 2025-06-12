#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUF_SIZE 1024

int sockfd;
char id_str[32];

void sigint_handler(int sig) {
    (void)sig;
    send(sockfd, "STOP\n", 5, 0);
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <id> <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }
    signal(SIGINT, sigint_handler);
    strncpy(id_str, argv[1], sizeof(id_str) - 1);
    id_str[sizeof(id_str) - 1] = '\0';

    const char *srv_ip = argv[2];
    int port = atoi(argv[3]);

    struct sockaddr_in srv;
    socklen_t srv_len = sizeof(srv);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(port);
    inet_pton(AF_INET, srv_ip, &srv.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&srv, srv_len) < 0) {
        perror("connect");
        exit(1);
    }

    char reg[64];
    snprintf(reg, sizeof(reg), "REG %s\n", id_str);
    send(sockfd, reg, strlen(reg), 0);

    fd_set readfds;
    char buf[BUF_SIZE];

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sockfd, &readfds);
        int maxfd = sockfd;
        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(0, &readfds)) {
            if (!fgets(buf, sizeof(buf), stdin))
                continue;

            if (strncmp(buf, "/list", 5) == 0) {
                send(sockfd, "LIST\n", 5, 0);

            } else if (strncmp(buf, "/all ", 5) == 0) {
                char msg[BUF_SIZE];
                snprintf(msg, sizeof(msg), "2ALL %s", buf + 5);
                send(sockfd, msg, strlen(msg), 0);

            } else if (strncmp(buf, "/one ", 5) == 0) {
                char *space = strchr(buf + 5, ' ');
                if (space) {
                    *space = '\0';
                    char *dst = buf + 5;
                    char *msg = space + 1;
                    char cmd[BUF_SIZE];
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
                    snprintf(cmd, sizeof(cmd), "2ONE %s %s", dst, msg);
    #pragma GCC diagnostic pop
                    send(sockfd, cmd, strlen(cmd), 0);
                }

            } else if (strncmp(buf, "/stop", 5) == 0) {
                send(sockfd, "STOP\n", 5, 0);
                break;
            }
        }

        /* socket ready? */
        if (FD_ISSET(sockfd, &readfds)) {
            int r = recv(sockfd, buf, sizeof(buf) - 1, 0);
            if (r <= 0) {
                printf("Server disconnected\n");
                break;
            }
            buf[r] = '\0';

            if (strcmp(buf, "PING\n") == 0) {
                send(sockfd, "ALIVE\n", 6, 0);
            } else {
                printf("%s", buf);
            }
        }
    }

    close(sockfd);
    return 0;
}
