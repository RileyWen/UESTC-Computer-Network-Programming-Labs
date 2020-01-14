#ifndef LIB_LAB_4_H
#define LIB_LAB_4_H

#include "mylib.h"

#define LEN_MSG 112
#define LEN_CLIENT_NAME 16

#define SERVER_MSG_CLI_LIST 0x1
#define SERVER_MSG_SEND 0x2
#define SERVER_MSG_HEARTBEAT 0x3

typedef struct {
    uint32_t ip;
    uint16_t port;
} cli_addr_t;

typedef struct {
    short list_len;
    cli_addr_t cli_addr_list[21];
} server_msg_cli_list_t;

typedef struct {
    char client_name[LEN_CLIENT_NAME];
    char msg_content[LEN_MSG];
} server_msg_send_t;

typedef struct {
    short type;
    union {
        server_msg_cli_list_t cli_list;
        server_msg_send_t msg_sent;
    };
} server_msg_t;

#define CLIENT_MSG_ONLINE 0x1
#define CLIENT_MSG_BROADCAST 0x2
#define CLIENT_MSG_OFFLINE 0x3
#define CLIENT_MSG_SENDTO 0x4
#define CLIENT_MSG_HEARTBEAT_ACK 0x5

typedef struct {
    short type;
    char client_name[LEN_CLIENT_NAME];
    char msg_content[LEN_MSG];
} client_msg_t;

typedef struct _list_node_t {
    char client_name[LEN_CLIENT_NAME];
    struct sockaddr_in client_addr;
    uint32_t unacked_heartbeat;
    struct _list_node_t *next;
} client_list_node_t;

#define for_each_in_client_list(p) while (p = p->next, p)

client_list_node_t *client_list_create();

void client_list_push_front(client_list_node_t *head, const char *client_name,
                            const struct sockaddr_in *client_addr);

void client_list_erase_node(client_list_node_t *head, const char *client_name);

#endif