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
    char line_buf[MAXLINE];
    int i;
    struct sockaddr_in cli_addr_temp;

    snprintf(client_name, LEN_CLIENT_NAME, "%d", getpid());
    bzero(&cli_addr_temp, sizeof(struct sockaddr_in));

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
            Readline(STDIN_FILENO, line_buf, MAXLINE);

            char *cp = line_buf;
            while (*cp != '\n') {
                if (*cp == ' ')
                    *cp = '\0';
            }

            cp = line_buf;
            if (strcmp("sendto", cp) == 0) {
                cp += 7;

            } else if (strcmp("showclients", cp) == 0) {
                cp += 12;
            }

            client_msg.type = CLIENT_MSG_BROADCAST;
            Sendto(client_fd, &client_msg, sizeof(client_msg_t), 0,
                   (pSA)&server_addr, sizeof(struct sockaddr_in));
        }
    } else {
        Close(STDIN_FILENO);
        while (1) {
            Recvfrom(client_fd, &server_msg, sizeof(server_msg_t), 0, NULL,
                     NULL);

            switch (server_msg.type) {
            case SERVER_MSG_CLI_LIST:
                for (i = 0; i < server_msg.cli_list.list_len; i++) {
                    cli_addr_temp.sin_family = AF_INET;
                    cli_addr_temp.sin_addr.s_addr =
                        server_msg.cli_list.cli_addr_list[i].ip;
                    cli_addr_temp.sin_port =
                        server_msg.cli_list.cli_addr_list[i].port;
                }
                break;

            case SERVER_MSG_SEND:
                printf("[Client %s]: %s", server_msg.msg_sent.client_name,
                       server_msg.msg_sent.msg_content);
                break;

            case SERVER_MSG_HEARTBEAT:
#ifdef DEBUG
                printf("Receive an heartbeat from server. ACKing...\n");
#endif
                client_msg.type = CLIENT_MSG_HEARTBEAT_ACK;
                Sendto(client_fd, &client_msg, sizeof(client_msg_t), 0,
                       (pSA)&server_addr, sizeof(struct sockaddr_in));

            default:
                break;
            }
        }
    }
}