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

void stdin_handler(int client_fd, client_list_node_t *client_list_head) {
    char line_buf[MAXLINE];
    client_msg_t client_msg;
    server_msg_t server_msg;
    client_list_node_t *p;
    char client_name_to_be_sent[LEN_CLIENT_NAME];

    Readline(STDIN_FILENO, line_buf, MAXLINE);

    char *cp = line_buf;
    while (*cp != '\n') {
        if (*cp == ' ')
            *cp = '\0';

        cp++;
    }

    cp = line_buf;
    if (strcmp("sendto", cp) == 0) {
        cp += 7;
        strncpy(client_name_to_be_sent, cp, LEN_CLIENT_NAME);

        while (*cp != '\0')
            cp++;
        cp++;

#ifdef DEBUG
        printf("Trying sending to %s: %s", client_name_to_be_sent, cp);
#endif
        p = client_list_head;
        for_each_in_client_list(p) {
            if (strncmp(p->client_name, client_name_to_be_sent,
                        LEN_CLIENT_NAME) == 0)
                break;
        }

        if (p) {
            client_msg.type = CLIENT_MSG_SENDTO;
            strncpy(client_msg.client_name, client_name, LEN_CLIENT_NAME);
            strncpy(client_msg.dest_name, client_name_to_be_sent,
                    LEN_CLIENT_NAME);
            strncpy(client_msg.msg_content, cp, LEN_MSG);
            Sendto(client_fd, &client_msg, sizeof(client_msg_t), 0,
                   (pSA)&server_addr, sizeof(struct sockaddr_in));
        }

    } else if (strcmp("showclients\n", cp) == 0) {
        printf("Available clients:\n");

        p = client_list_head;
        for_each_in_client_list(p) { printf("\t%s\n", p->client_name); }
        printf("\n");
    } else if (strcmp("dst", cp) == 0) { // Directly send to another client
        cp += 4;
        strncpy(client_name_to_be_sent, cp, LEN_CLIENT_NAME);

        while (*cp != '\0')
            cp++;
        cp++;

#ifdef DEBUG
        printf("Trying directly sending to %s: %s", client_name_to_be_sent, cp);
#endif
        p = client_list_head;
        for_each_in_client_list(p) {
            if (strncmp(p->client_name, client_name_to_be_sent,
                        LEN_CLIENT_NAME) == 0)
                break;
        }

        if (p) {
            server_msg.type = SERVER_MSG_DIRECT_COMM;
            strncpy(server_msg.msg_sent.client_name, client_name,
                    LEN_CLIENT_NAME);
            strncpy(server_msg.msg_sent.msg_content, cp, LEN_MSG);
            Sendto(client_fd, &server_msg, sizeof(server_msg_t), 0,
                   (pSA)&p->client_addr, sizeof(struct sockaddr_in));
        }
    }
}

void client_fd_handler(int client_fd, client_list_node_t **p_client_list_head) {
    int i;
    client_msg_t client_msg;
    server_msg_t server_msg;
    struct sockaddr_in cli_addr_temp;

    bzero(&cli_addr_temp, sizeof(struct sockaddr_in));

    Recvfrom(client_fd, &server_msg, sizeof(server_msg_t), 0, NULL, NULL);

    switch (server_msg.type) {
    case SERVER_MSG_SEND:
        printf("[Client %s | SERVER FORWARDING]: %s", server_msg.msg_sent.client_name,
               server_msg.msg_sent.msg_content);
        break;

    case SERVER_MSG_HEARTBEAT:
#ifdef DEBUG
        // printf("Receive an heartbeat from server. ACKing...\n");
#endif
        // Store client lists carried by Heartbeat
        if (*p_client_list_head)
            client_list_free(*p_client_list_head);
        *p_client_list_head = client_list_create();

        for (i = 0; i < server_msg.cli_list.list_len; i++) {
            cli_addr_temp.sin_family = AF_INET;
            cli_addr_temp.sin_addr.s_addr =
                server_msg.cli_list.cli_addr_list[i].ip;
            cli_addr_temp.sin_port = server_msg.cli_list.cli_addr_list[i].port;

            // printf("%hu\n", ntohs(cli_addr_temp.sin_port));

            client_list_push_front(
                *p_client_list_head,
                server_msg.cli_list.cli_addr_list[i].client_name,
                &cli_addr_temp);
        }

        client_msg.type = CLIENT_MSG_HEARTBEAT_ACK;
        strncpy(client_msg.client_name, client_name, LEN_CLIENT_NAME);
        Sendto(client_fd, &client_msg, sizeof(client_msg_t), 0,
               (pSA)&server_addr, sizeof(struct sockaddr_in));
        
        break;

    case SERVER_MSG_DIRECT_COMM:
        printf("[Client %s | DIRECT COMM]: %s", server_msg.msg_sent.client_name,
               server_msg.msg_sent.msg_content);
        break;

    default:
        break;
    }
}

int main(int argc, char **argv) {
    client_msg_t client_msg;
    client_list_node_t *client_list_head;

    fd_set rset, readyset;

    client_list_head = NULL;
    snprintf(client_name, LEN_CLIENT_NAME, "%d", getpid());

    bzero(&server_addr, sizeof(struct sockaddr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)strtoul(argv[2], NULL, 10));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    client_fd = open_udp_clientfd();
    FD_ZERO(&rset);
    FD_SET(client_fd, &rset);
    FD_SET(STDIN_FILENO, &rset);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, sigint_handler);

    client_msg.type = CLIENT_MSG_ONLINE;
    strncpy(client_msg.client_name, client_name, LEN_CLIENT_NAME);
    sendto(client_fd, &client_msg, sizeof(client_msg_t), 0, (pSA)&server_addr,
           sizeof(struct sockaddr_in));

    while (1) {
        readyset = rset;
        Select(client_fd + 1, &readyset, NULL, NULL, NULL);

        if (FD_ISSET(client_fd, &readyset)) {
            client_fd_handler(client_fd, &client_list_head);
        }

        if (FD_ISSET(STDIN_FILENO, &readyset)) {
            stdin_handler(client_fd, client_list_head);
        }
    }
}