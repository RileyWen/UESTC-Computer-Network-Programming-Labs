#include <stdint.h> /* Definition of uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#define unix_error(msg)                                                      \
    do {                                                                       \
        perror(msg);                                                           \
        exit(EXIT_FAILURE);                                                    \
    } while (0)

#define Select select

int generate_timer_fd(time_t interval_sec) {
    int fd;
    struct timespec now;
    struct itimerspec new_value;

    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
        unix_error("clock_getime error");

    new_value.it_value.tv_sec = now.tv_sec + 1;
    new_value.it_value.tv_nsec = now.tv_nsec;

    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_nsec = 0;

    fd = timerfd_create(CLOCK_REALTIME, 0);
    if (fd == -1)
        unix_error("timerfd_create");
    
    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
        unix_error("timerfd_settime");

    return fd;
}

int main() {
    fd_set rset, readyset;
    int timer_fd, nready;
    uint64_t exp;

    timer_fd = generate_timer_fd(1);

    FD_ZERO(&rset);
    FD_SET(timer_fd, &rset);

    while (1) {
        readyset = rset;
        nready = Select(timer_fd+1, &readyset, NULL, NULL, NULL);

        if (FD_ISSET(timer_fd, &readyset)) {
            read(timer_fd, &exp, sizeof(uint64_t));
            printf("Timer has been triggered %llu times!\n", exp);
        }
    }

    return 0;    
}