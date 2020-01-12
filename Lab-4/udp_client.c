#include "lib_lab_4.h"
#include "mylib.h"

char client_name[LEN_CLIENT_NAME];
struct sockaddr_in server_addr;
int client_fd;

void sigint_handler(int signum) {
    client_msg_t client_msg;

    printf("\nReceived SIGINT! Informing server...\n");

    client_msg.type = CLIENT_MSG_OFFLINE;
    strncpy(client_msg.client_name, client_name, LEN_CLIENT_NAME);
    sendto(client_fd, &client_msg, sizeof(client_msg_t), 0, (pSA)&server_addr,
           sizeof(struct sockaddr_in));

    printf("Exiting...\n");
    exit(0);
}

int main(int argc, char **argv) {
    server_msg_t server_msg;
    client_msg_t client_msg;

    snprintf(client_name, LEN_CLIENT_NAME, "%d", getpid());

    bzero(&server_addr, sizeof(struct sockaddr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)strtoul(argv[2], NULL, 10));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    client_fd = open_udp_clientfd();

    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, sigint_handler);

    client_msg.type = CLIENT_MSG_ONLINE;
    strncpy(client_msg.client_name, client_name, LEN_CLIENT_NAME);
    sendto(client_fd, &client_msg, sizeof(client_msg_t), 0, (pSA)&server_addr,
           sizeof(struct sockaddr_in));

    if (Fork() == 0) {
        signal(SIGINT, SIG_DFL);
        while (1) {
            client_msg.type = CLIENT_MSG_BROADCAST;
            Readline(STDIN_FILENO, client_msg.msg_content, LEN_MSG);

            Sendto(client_fd, &client_msg, sizeof(client_msg_t), 0,
                   (pSA)&server_addr, sizeof(struct sockaddr_in));
        }
    } else {
        Close(STDIN_FILENO);
        while (1) {
            Recvfrom(client_fd, &server_msg, sizeof(server_msg_t), 0, NULL,
                     NULL);
            printf("[Client %s]: %s", server_msg.client_name,
                   server_msg.msg_content);
        }
    }
}