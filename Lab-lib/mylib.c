#include "mylib.h"

ssize_t /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; /* and call write() again */
            else
                return (-1); /* error */
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}
/* end writen */

void Write(int fd, void *ptr, size_t nbytes) {
    if (writen(fd, ptr, nbytes) != nbytes)
        err_quit("writen error");
}

/* Slow version of readline for convenience */
ssize_t Readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break; /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return n - 1; /* EOF, n - 1 bytes were read */
        } else
            return -1; /* error, errno set by read() */
    }

    *ptr = 0; /* null terminate like fgets() */
    return n;
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {
    int n = sendto(sockfd, buf, len, flags, dest_addr, addrlen);

    if (n < 0)
        unix_error("sendto error");
    return n;
}

ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen) {
    int n = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);

    if (n < 0)
        unix_error("recvfrom error");
    return n;
}

void err_quit(const char *err_info) {
    fprintf(stderr, "%s\n", err_info);
    exit(1);
}

void unix_error(char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void posix_error(int err_code, char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(err_code));
    exit(1);
}

int open_tcp_clientfd(char *ipaddr, char *port) {
    struct sockaddr_in client_addr;
    int sockfd;

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&client_addr, sizeof(struct sockaddr_in));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons((uint16_t)strtoul(port, NULL, 10));
    inet_pton(AF_INET, ipaddr, &client_addr.sin_addr);

    Connect(sockfd, (pSA)&client_addr, sizeof(client_addr));

    return sockfd;
}

int open_tcp_serverfd(char *ipaddr, char *port) {
    struct sockaddr_in server_addr;
    int listenfd, optval = 1;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ipaddr, &server_addr.sin_addr);
    server_addr.sin_port = htons((uint16_t)strtoul(port, NULL, 10));

    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
               sizeof(int));

    Bind(listenfd, (pSA)&server_addr, sizeof(server_addr));

    Listen(listenfd);

    return listenfd;
}

int open_udp_serverfd(char *ipaddr, char *port) {
    struct sockaddr_in server_addr;
    int listenfd, optval = 1;

    listenfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ipaddr, &server_addr.sin_addr);
    server_addr.sin_port = htons((uint16_t)strtoul(port, NULL, 10));

    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
               sizeof(int));

    Bind(listenfd, (pSA)&server_addr, sizeof(server_addr));

    return listenfd;
}

int open_udp_clientfd() { return Socket(AF_INET, SOCK_DGRAM, 0); }

void server_echo_routine(int connfd) {
    ssize_t n;
    char buf[256];

again:
    while ((n = read(connfd, buf, 256)) > 0)
        Write(connfd, buf, n);

    if (n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
        unix_error("read error");
    else if (n == 0)   // EOF occurs, close the fd.
        Close(connfd); // Closing fd automatically
                       // remove it from epoll fs.
}

void server_time_routine(int connfd) {
    ssize_t n;
    char buf[MAXLINE];
    time_t timer;
    struct tm *tm_info;

    while ((n = Readline(connfd, buf, MAXLINE)) > 0) {
        if (strncmp(buf, "show time\n", MAXLINE) == 0) {
            time(&timer);
            tm_info = localtime(&timer);
            strftime(buf, 26, "%Y-%m-%d %H:%M:%S\n", tm_info);

            Write(connfd, buf, strnlen(buf, MAXLINE));
        }
    }

    Close(connfd);
}

void Listen(int fd) {
    if (listen(fd, MAX_LISTEN_Q) < 0)
        unix_error("listen error");
}

int Socket(int family, int type, int protocol) {
    int n;
    if ((n = socket(family, type, protocol)) < 0)
        unix_error("socket error");
    return n;
}

void Bind(int fd, const pSA sa, socklen_t salen) {
    if (bind(fd, sa, salen) < 0)
        unix_error("bind error");
}

void Connect(int fd, const pSA sa, socklen_t len) {
    if (connect(fd, sa, len) < 0)
        unix_error("connect error");
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr) {
    int n;

again:
    if ((n = accept(fd, sa, salenptr)) < 0) {
        if (errno == EPROTO || errno == ECONNABORTED)
            goto again;
        else
            unix_error("accept error");
    }
    return n;
}

void Setsockopt(int sockfd, int level, int optname, const void *optval,
                socklen_t optlen) {
    if (setsockopt(sockfd, level, optname, optval, optlen) < 0)
        unix_error("setsockopt error");
}

void Close(int fd) {
    if (close(fd) < 0)
        unix_error("close error");
}

pid_t Fork(void) {
    pid_t pid = fork();
    if (pid < 0)
        unix_error("fork error");

    return pid;
}

__sighandler_t Signal(int signum, __sighandler_t handler) {
    __sighandler_t sh;
    if ((sh = signal(signum, handler)) == SIG_ERR)
        unix_error("signal error");

    return sh;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout) {
    int n;

    if ((n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
        unix_error("select error");
    return n; /* can return 0 on timeout */
}

int Epoll_create1(int flags) {
    int fd = epoll_create1(flags);
    if (fd == -1)
        unix_error("epoll_create1 error");
    return fd;
}

void Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    if (epoll_ctl(epfd, op, fd, event) != 0)
        unix_error("epoll_ctl error");
}

int Epoll_wait(int epfd, struct epoll_event *events, int maxevents,
               int timeout) {
    int nready = epoll_wait(epfd, events, maxevents, timeout);
    if (nready < 0)
        unix_error("epoll_wait_error");

    return nready;
}

void Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg) {
    int rc;

    if ((rc = pthread_create(thread, attr, start_routine, arg)) != 0)
        posix_error(rc, "pthread_create error");
}

void Pthread_detach(pthread_t thread) {
    int rc;

    if ((rc = pthread_detach(thread)) != 0)
        posix_error(rc, "pthread_detach error");
}