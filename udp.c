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
    struct timeval timeout = {5, 0};
    char port[6];
    int fd;
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
    /* Em UDP, connect fixa o DS como destino e filtra respostas de outras origens. */
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0 ||
        connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    freeaddrinfo(res);

    do {
        nsent = send(fd, message, length, 0);
    } while (nsent < 0 && errno == EINTR);
    if (nsent < 0 || (size_t)nsent != length) {
        close(fd);
        return -1;
    }

    /* Se o buffer ficar cheio, a resposta pode ter sido cortada. */
    do {
        nread = recv(fd, response, (size_t)response_size, 0);
    } while (nread < 0 && errno == EINTR);
    if (nread <= 0) {
        response[0] = '\0';
        close(fd);
        return -1;
    }
    if (nread >= response_size ||
        !validate_protocol_message(response, (size_t)nread)) {
        response[0] = '\0';
        close(fd);
        return -2;
    }
    response[nread] = '\0';
    close(fd);
    return 0;
}
