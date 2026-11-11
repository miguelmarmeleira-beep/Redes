#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "tcp.h"
#include "validation.h"

int send_tcp_request(const AppConfig *config, const char *message,
                     char **response)
{
    struct addrinfo hints, *res;
    struct timeval timeout = {5, 0};
    char port[6], *buffer;
    char *newline = NULL;
    int fd, result = -1;
    size_t sent = 0, length = strlen(message);
    size_t used = 0, capacity = 4096;

    *response = NULL;
    snprintf(port, sizeof(port), "%d", config->ds_port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(config->ds_ip, port, &hints, &res) != 0) {
        return -1;
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0 ||
        connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    freeaddrinfo(res);

    /* TCP pode enviar apenas parte do pedido em cada chamada. */
    while (sent < length) {
        ssize_t n = send(fd, message + sent, length - sent, MSG_NOSIGNAL);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            close(fd);
            return -1;
        }
        sent += (size_t)n;
    }

    buffer = malloc(capacity);
    if (buffer == NULL) {
        close(fd);
        return -1;
    }
    while (newline == NULL) {
        ssize_t n;
        char *grown;

        if (used == capacity - 1) {
            if (capacity > SIZE_MAX / 2) {
                break;
            }
            grown = realloc(buffer, capacity * 2);
            if (grown == NULL) {
                break;
            }
            buffer = grown;
            capacity *= 2;
        }
        n = read(fd, buffer + used, capacity - used - 1);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n < 0) {
            break;
        }
        if (n == 0) {
            result = -2; /* A ligacao fechou antes do fim da resposta. */
            break;
        }
        newline = memchr(buffer + used, '\n', (size_t)n);
        used += (size_t)n;
    }
    if (newline != NULL) {
        if (newline != buffer + used - 1 ||
            !validate_protocol_message(buffer, used)) {
            result = -2;
        } else {
            buffer[used] = '\0';
            *response = buffer;
            buffer = NULL; /* O chamador passa a libertar a resposta. */
            result = 0;
        }
    }
    free(buffer);
    close(fd);
    return result;
}
