#ifndef MYLIB_H
#define MYLIB_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_LISTEN_Q 1024
#define MAXLINE 1024

typedef struct sockaddr *pSA;

/* Print error info and quit */
void err_quit(const char *err_info);

/* Error outputs */
void unix_error(char *msg);

void posix_error(int err_code, char *msg);

/* Wrapper for I/O functions */
void Write(int fd, void *ptr, size_t nbytes);

ssize_t Readline(int fd, void *vptr, size_t maxlen);

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);

/* Wrapper for opening socket */
int open_tcp_clientfd(char *ipaddr, char *port);

int open_tcp_serverfd(char *ipaddr, char *port);

int open_udp_serverfd(char *ipaddr, char *port);

int open_udp_clientfd();

/* Server routine */
void server_echo_routine(int connfd_arg);

void server_time_routine(int connfd);

/* Wrapper functions that handle unix errors */
int Socket(int family, int type, int protocol);

void Listen(int fd);

void Bind(int fd, const pSA sa, socklen_t len);

void Connect(int fd, const pSA sa, socklen_t len);

int Accept(int fd, pSA sa, socklen_t *salenptr);

void Setsockopt(int sockfd, int level, int optname, const void *optval,
                socklen_t optlen);

void Close(int fd);

pid_t Fork(void);

__sighandler_t Signal(int signum, __sighandler_t handler);

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout);

int Epoll_create1(int flags);

void Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

int Epoll_wait(int epfd, struct epoll_event *events, int maxevents,
               int timeout);

/* Wrapper functions that handle POSIX errors */
void Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg);

/* pthread_self() always succeeds. No need to handle errors. */

void Pthread_detach(pthread_t thread);

#endif