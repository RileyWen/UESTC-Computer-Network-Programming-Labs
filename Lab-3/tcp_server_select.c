
#include "mylib.h"

int main(int argc, char **argv) {
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];

    if (argc == 3)
        listenfd = open_tcp_serverfd(argv[1], argv[2]);
    else
        err_quit("usage: tcpserv01 <port>");

    maxfd = listenfd; 
    maxi = -1;        
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1; 
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    
    for (;;) {
        rset = allset; 
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) { 
            connfd = Accept(listenfd, NULL, NULL);

            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                    client[i] = connfd; 
                    break;
                }
            if (i == FD_SETSIZE)
                err_quit("too many clients");

            FD_SET(connfd, &allset); 
            if (connfd > maxfd)
                maxfd = connfd; 
            if (i > maxi)
                maxi = i; 

            if (--nready <= 0)
                continue; 
        }

        for (i = 0; i <= maxi; i++) { 
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ((n = Readline(sockfd, buf, MAXLINE)) == 0) {
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else 
                    Write(sockfd, buf, n);

                if (--nready <= 0)
                    break; 
            }
        }
    }
}

