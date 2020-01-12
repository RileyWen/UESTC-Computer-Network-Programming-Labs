#include "mylib.h"

int main(int argc, char **argv) {
    int listenfd, connfd;
    pid_t childpid;

    // Prevent child process from becoming zombies
    Signal(SIGCHLD, SIG_IGN);

    if (argc == 3)
        listenfd = open_tcp_serverfd(argv[1], argv[2]);
    else
        err_quit("usage: tcp_server_proc <port>");

    for (;;) {
        connfd = Accept(listenfd, NULL, NULL);

        if ((childpid = Fork()) == 0) {  /* child process */
            Close(listenfd);             /* close listening socket */
            server_time_routine(connfd); /* Time Routine (It handles EOF) */ 
            exit(0);
        }
        Close(connfd); /* parent closes connected socket */
    }
}