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
    struct timeval timeout = {5, 0};
    char port[6];
    int fd;
    ssize_t nsent, nread;
    size_t length = strlen(message);

    if (response_size < 2)
        return -1;
    response[0] = '\0';

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;

    snprintf(port, sizeof(port), "%d", config->ds_port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(config->ds_ip, port, &hints, &res) != 0) {
        close(fd);
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    nsent = sendto(fd, message, length, 0, res->ai_addr, res->ai_addrlen);

    if (nsent < 0 || (size_t)nsent != length) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    nread = recvfrom(fd, response, (size_t)response_size, 0, NULL, NULL);
    if (nread <= 0) {
        response[0] = '\0';
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    if (nread >= response_size ||
        !validate_protocol_message(response, (size_t)nread)) {
        response[0] = '\0';
        freeaddrinfo(res);
        close(fd);
        return -2;
    }

    response[nread] = '\0';
    freeaddrinfo(res);
    close(fd);
    return 0;
}