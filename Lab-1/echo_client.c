#include "mylib.h"

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 3)
        err_quit("usage: echo_client <IP Address> <Port>");

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((uint16_t)strtoul(argv[2], NULL, 10));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    Connect(sockfd, (pSA)&servaddr, sizeof(servaddr));

    while (1) {
        char line_buf[256];
        if (fgets(line_buf, 256, stdin) == NULL) {
            line_buf[0] = '\n';
            line_buf[1] = '\0';
        }

        Write(sockfd, line_buf, strlen(line_buf));

        Readline(sockfd, line_buf, 256);

        printf("%s", line_buf);
    }

    return 0;
}
