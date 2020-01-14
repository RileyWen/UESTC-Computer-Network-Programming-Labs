#include "lib_lab_4.h"

client_list_node_t *client_list_create() {
    client_list_node_t *head;

    head = malloc(sizeof(client_list_node_t));
    head->next = NULL;

    return head;
}

void client_list_push_front(client_list_node_t *head, const char *client_name,
                            const struct sockaddr_in *client_addr) {
    client_list_node_t *old_next;
    old_next = head->next;

    head->next = malloc(sizeof(client_list_node_t));
    head->next->next = old_next;

    strncpy(head->next->client_name, client_name, LEN_CLIENT_NAME);
    head->next->client_addr = *client_addr;
    head->next->unacked_heartbeat = 0;
}

void client_list_erase_node(client_list_node_t *head, const char *client_name) {
    client_list_node_t *p, *p_prev;

    p = head;
    while (p_prev = p, p = p->next, p)
        if (strncmp(client_name, p->client_name, LEN_CLIENT_NAME) == 0) {
            p_prev->next = p->next;
            free(p);
            break;
        }
}