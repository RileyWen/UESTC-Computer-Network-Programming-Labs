#include "mylib.h"

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    signal(SIGCHLD, SIG_IGN);

    if (argc != 3)
        err_quit("usage: time_client <IP Address> <Port>");

    sockfd = open_tcp_clientfd(argv[1], argv[2]);

    char line_buf[MAXLINE];
    if (Fork() == 0) {
        while (1) {
            Readline(STDIN_FILENO, line_buf, MAXLINE);

            Write(sockfd, line_buf, strlen(line_buf));
        }
    } else {
        Close(STDIN_FILENO);
        while (1) {
            Readline(sockfd, line_buf, MAXLINE);
            printf("%s", line_buf);
        }
    }

    return 0;
}
