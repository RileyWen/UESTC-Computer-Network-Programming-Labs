#include "mylib.h"

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    if (argc != 3) 
        err_quit("usage: echo_server <IPaddress> <Port>");

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    servaddr.sin_port = htons((uint16_t)strtoul(argv[2], NULL, 10));

    Bind(listenfd, (pSA)&servaddr, sizeof(servaddr));

    Listen(listenfd);

    for (;;) {
        clilen = sizeof(cliaddr);
        connfd = Accept(listenfd, (pSA)&cliaddr, &clilen);

        server_echo_routine(connfd); // Echo routine has already
                                     // handled the EOF caused by
                                     // closed connection
    }
}
