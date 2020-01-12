#include "lib_lab_4.h"
#include "mylib.h"

int main(int argc, char **argv) {
    int serv_fd, recv_len;
    socklen_t cli_addr_len;
    struct sockaddr_in cli_addr;
    char addr_repr[16];
    client_msg_t client_msg;
    server_msg_t server_msg;
    client_list_node_t *client_list_head, *p;

    client_list_head = client_list_create();

    serv_fd = open_udp_serverfd(argv[1], argv[2]);

    while (1) {
        cli_addr_len = sizeof(struct sockaddr_in);
        recv_len = Recvfrom(serv_fd, &client_msg, sizeof(client_msg_t), 0,
                            (pSA)&cli_addr, &cli_addr_len);
        inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, addr_repr, 16);

        if (recv_len != sizeof(client_msg_t)) {
            fprintf(stderr,
                    "[Error] Received an malformed message "
                    "from %s, ignoring it.\n",
                    addr_repr);
            continue;
        }

        switch (client_msg.type) {
        case CLIENT_MSG_ONLINE:
            printf("[Client %s] Online at %s:%hu\n", client_msg.client_name,
                   addr_repr, ntohs(cli_addr.sin_port));

            client_list_push_front(client_list_head, client_msg.client_name,
                                   &cli_addr);
            break;

        case CLIENT_MSG_OFFLINE:
            printf("[Client %s] Offline!\n", client_msg.client_name);

            client_list_erase_node(client_list_head, client_msg.client_name);
            break;

        case CLIENT_MSG_BROADCAST:
            printf("[Client %s] Broadcasting msg: %s", client_msg.client_name,
                   client_msg.msg_content);

            p = client_list_head;
            for_each_in_client_list(p) {
                if (strncmp(p->client_name, client_msg.client_name,
                            LEN_CLIENT_NAME)) {

                    strncpy(server_msg.client_name, p->client_name,
                            LEN_CLIENT_NAME);
                    strncpy(server_msg.msg_content, client_msg.msg_content,
                            LEN_MSG);

                    Sendto(serv_fd, &server_msg, sizeof(server_msg_t), 0,
                           (pSA) & (p->client_addr),
                           sizeof(struct sockaddr_in));
                }
            }
            break;

        default:
            break;
        }
    }

    return 0;
}