#include "lib_lab_4.h"
#include "mylib.h"

int generate_timer_fd(time_t interval_sec) {
    int fd;
    struct timespec now;
    struct itimerspec new_value;

    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
        unix_error("clock_getime error");

    new_value.it_value.tv_sec = now.tv_sec + 1;
    new_value.it_value.tv_nsec = now.tv_nsec;

    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_nsec = 0;

    fd = timerfd_create(CLOCK_REALTIME, 0);
    if (fd == -1)
        unix_error("timerfd_create");

    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
        unix_error("timerfd_settime");

    return fd;
}

void client_msg_handler(int serv_fd, client_list_node_t *client_list_head) {
    int recv_len;
    client_msg_t client_msg;
    server_msg_t server_msg;
    client_list_node_t *p;
    socklen_t cli_addr_len;
    struct sockaddr_in cli_addr;
    char addr_repr[16];

    cli_addr_len = sizeof(struct sockaddr_in);
    recv_len = Recvfrom(serv_fd, &client_msg, sizeof(client_msg_t), 0,
                        (pSA)&cli_addr, &cli_addr_len);
    inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, addr_repr, 16);

    if (recv_len != sizeof(client_msg_t)) {
        fprintf(stderr,
                "[Error] Received an malformed message "
                "from %s, ignoring it.\n",
                addr_repr);
        return;
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

                server_msg.type = SERVER_MSG_SEND;
                strncpy(server_msg.msg_sent.client_name, p->client_name,
                        LEN_CLIENT_NAME);
                strncpy(server_msg.msg_sent.msg_content, client_msg.msg_content,
                        LEN_MSG);

                Sendto(serv_fd, &server_msg, sizeof(server_msg_t), 0,
                       (pSA) & (p->client_addr), sizeof(struct sockaddr_in));
            }
        }
        break;

    default:
        break;
    }
}

void send_heartbeat(int serv_fd, client_list_node_t *client_list_head) {
    server_msg_t server_msg;
    server_msg.type = SERVER_MSG_HEARTBEAT;

#ifdef DEBUG
    printf("sending heartbeat to all clients...\n");
#endif

    client_list_node_t *p = client_list_head;
    for_each_in_client_list(p) {
        Sendto(serv_fd, &server_msg, sizeof(server_msg_t), 0,
               (pSA) & (p->client_addr), sizeof(struct sockaddr_in));
    }
}

int main(int argc, char **argv) {
    int serv_fd, timer_fd;
    client_list_node_t *client_list_head;
    fd_set rset, ready_set;

    client_list_head = client_list_create();
    serv_fd = open_udp_serverfd(argv[1], argv[2]);
    timer_fd = generate_timer_fd(1);

    FD_ZERO(&rset);
    FD_SET(serv_fd, &rset);
    FD_SET(timer_fd, &rset);

    while (1) {
        ready_set = rset;
        Select(((serv_fd > timer_fd) ? serv_fd : timer_fd) + 1, &ready_set,
               NULL, NULL, NULL);

        if (FD_ISSET(timer_fd, &ready_set))
            send_heartbeat(serv_fd, client_list_head);
        if (FD_ISSET(serv_fd, &ready_set))
            client_msg_handler(serv_fd, client_list_head);
    }

    return 0;
}