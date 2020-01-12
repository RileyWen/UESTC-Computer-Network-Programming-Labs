#ifndef LIB_LAB_4_H
#define LIB_LAB_4_H

#include "mylib.h"

#define LEN_MSG 64
#define LEN_CLIENT_NAME 16

typedef struct {
    char client_name[LEN_CLIENT_NAME];
    char msg_content[LEN_MSG];
} server_msg_t;

#define CLIENT_MSG_ONLINE 0x1
#define CLIENT_MSG_BROADCAST 0x2
#define CLIENT_MSG_OFFLINE 0x3

typedef struct {
    char type;
    char client_name[LEN_CLIENT_NAME];
    char msg_content[LEN_MSG];
} client_msg_t;

typedef struct _list_node_t {
    char client_name[LEN_CLIENT_NAME];
    struct sockaddr_in client_addr;
    struct _list_node_t *next;
} client_list_node_t;

#define for_each_in_client_list(p) while (p = p->next, p)

client_list_node_t *client_list_create() ;

void client_list_push_front(client_list_node_t *head, const char *client_name,
                            const struct sockaddr_in *client_addr) ;

void client_list_erase_node(client_list_node_t *head, const char *client_name) ;

#endif