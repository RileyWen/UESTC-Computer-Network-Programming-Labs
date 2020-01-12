#include "mylib.h"

#define MAX_EVENTS 10

int main(int argc, char **argv) {
    struct epoll_event ev, events[MAX_EVENTS];
    int listenfd, connfd, nready, epollfd;
    char buf[MAXLINE];

    if (argc == 3)
        listenfd = open_tcp_serverfd(argv[1], argv[2]);
    else
        err_quit("usage: tcp_server_epoll <IP address> <port>");

    epollfd = Epoll_create1(0);

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    Epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    for (;;) {
        nready = Epoll_wait(epollfd, events, MAX_EVENTS, -1);

        for (int n = 0; n < nready; ++n) {
            if (events[n].data.fd == listenfd) {
                connfd = Accept(listenfd, NULL, NULL);

                ev.events = EPOLLIN;
                ev.data.fd = connfd;
                Epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev); 

            } else {
                connfd = events[n].data.fd;
                if ((n = Readline(connfd, buf, MAXLINE)) == 0) {
                    Close(connfd);
                } else 
                    Write(connfd, buf, n);
            }
        }
    }
}