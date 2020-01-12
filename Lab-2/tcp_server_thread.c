#include "mylib.h"

static void *doit(void *); /* each thread executes this function */

int main(int argc, char **argv) {
    int listenfd, connfd;
    pthread_t tid;

    if (argc == 3)
        listenfd = open_tcp_serverfd(argv[1], argv[2]);
    else
        err_quit("usage: tcp_server_thread <IP Address> <port>");

    for (;;) {
        connfd = Accept(listenfd, NULL, NULL);
        Pthread_create(&tid, NULL, &doit, &connfd);
    }
}

static void *doit(void *arg) {
    int connfd = *(int *)arg;

    Pthread_detach(pthread_self());

    server_time_routine(connfd); /* Time Routine (It handles EOF) */

	return NULL;
}
