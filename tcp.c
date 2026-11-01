#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "tcp.h"
#include "validation.h"

static long long monotonic_ms(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        return -1;
    return (long long)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

/* Um prazo total evita que um peer lento renove o timeout em cada byte. */
static int wait_socket(int fd, short events, long long deadline)
{
    struct pollfd pfd = {fd, events, 0};
    for (;;) {
        long long now = monotonic_ms();
        long long remaining = deadline - now;
        int result;
        if (now < 0 || remaining <= 0)
            return -1;
        result = poll(&pfd, 1, remaining > INT_MAX ? INT_MAX : (int)remaining);
        if (result < 0 && errno == EINTR)
            continue;
        if (result <= 0 || (pfd.revents & POLLNVAL))
            return -1;
        /* ERR/HUP sao tratados pela operacao seguinte, incluindo SO_ERROR. */
        return 0;
    }
}

int send_tcp_request(const AppConfig *config, const char *message,
                     char **response)
{
    struct addrinfo hints, *res;
    char port[6];
    int fd, flags, error = 0, result = -1;
    socklen_t error_size = sizeof(error);
    size_t written = 0, length = strlen(message);
    size_t total_read = 0, capacity = 4096;
    char *buffer = NULL;
    long long deadline;

    *response = NULL;
    snprintf(port, sizeof(port), "%d", config->ds_port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(config->ds_ip, port, &hints, &res) != 0)
        return -1;
    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        return -1;
    }
    flags = fcntl(fd, F_GETFL, 0);
    deadline = monotonic_ms();
    if (flags < 0 || deadline < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    deadline += 5000;
    error = connect(fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (error < 0 && errno != EINPROGRESS)
        goto done;
    if (wait_socket(fd, POLLOUT, deadline) != 0 ||
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &error_size) != 0 || error != 0)
        goto done;

    while (written < length) {
        ssize_t n;
        if (wait_socket(fd, POLLOUT, deadline) != 0)
            goto done;
        /* Evitar que uma ligacao interrompida termine o programa com SIGPIPE. */
        n = send(fd, message + written, length - written, MSG_NOSIGNAL);
        if (n < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            continue;
        if (n <= 0)
            goto done;
        written += (size_t)n;
    }
    buffer = malloc(capacity);
    if (buffer == NULL)
        goto done;
    for (;;) {
        ssize_t n;
        char *newline;
        if (total_read == capacity - 1) {
            char *grown;
            if (capacity > SIZE_MAX / 2)
                goto done;
            grown = realloc(buffer, capacity * 2);
            if (grown == NULL)
                goto done;
            buffer = grown;
            capacity *= 2;
        }
        if (wait_socket(fd, POLLIN, deadline) != 0)
            goto done;
        n = read(fd, buffer + total_read, capacity - total_read - 1);
        if (n < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            continue;
        if (n < 0)
            goto done;
        if (n == 0) {
            result = -2; /* EOF antes do terminador. */
            goto done;
        }
        newline = memchr(buffer + total_read, '\n', (size_t)n);
        total_read += (size_t)n;
        if (newline != NULL) {
            /* Uma ligacao corresponde a um pedido e a uma resposta. */
            if (newline != buffer + total_read - 1 ||
                !validate_protocol_message(buffer, total_read)) {
                result = -2;
                goto done;
            }
            buffer[total_read] = '\0';
            *response = buffer;
            buffer = NULL; /* O chamador passa a ser responsavel por free(). */
            result = 0;
            goto done;
        }
    }
done:
    free(buffer);
    close(fd);
    return result;
}
