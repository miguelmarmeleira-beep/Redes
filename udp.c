#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "udp.h"
#include "validation.h"

int send_udp_request(const AppConfig *config, const char *message,
                     char *response, int response_size)
{
    struct addrinfo hints, *res;
    struct timeval tv = {5, 0};
    struct iovec buffer;
    struct msghdr reply;
    char port[6];
    int fd, result = -1;
    ssize_t nread, nsent;
    size_t length = strlen(message);

    if (response_size < 2)
        return -1;
    response[0] = '\0';
    snprintf(port, sizeof(port), "%d", config->ds_port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(config->ds_ip, port, &hints, &res) != 0)
        return -1;
    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        return -1;
    }
    /* UDP connect filtra datagramas que nao venham do IP/porto do DS. */
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0 ||
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0 ||
        connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    freeaddrinfo(res);
    do {
        nsent = send(fd, message, length, 0);
    } while (nsent < 0 && errno == EINTR);
    if (nsent < 0 || (size_t)nsent != length)
        goto done;

    buffer.iov_base = response;
    buffer.iov_len = (size_t)response_size - 1;
    memset(&reply, 0, sizeof(reply));
    reply.msg_iov = &buffer;
    reply.msg_iovlen = 1;
    nread = recvmsg(fd, &reply, 0);
    if (nread <= 0)
        goto done;
    if ((reply.msg_flags & MSG_TRUNC) ||
        !validate_protocol_message(response, (size_t)nread)) {
        result = -2;
        goto done;
    }
    response[nread] = '\0';
    result = 0;
done:
    if (result != 0)
        response[0] = '\0';
    close(fd);
    return result;
}
