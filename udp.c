#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "udp.h"

int send_udp_request(const AppConfig *config, const char *message,
                     char *response, int response_size)
{
    struct addrinfo hints, *res;
    int fd, errcode;
    char port[6];
    ssize_t nread;

    /* Create UDP socket */
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd == -1)
        return -1;

    /* Convert DS port from int to string */
    snprintf(port, sizeof(port), "%d", config->ds_port);

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;       /* IPv4 */
    hints.ai_socktype = SOCK_DGRAM;  /* UDP socket */

    errcode = getaddrinfo(config->ds_ip, port, &hints, &res);

    if (errcode != 0) {
        close(fd);
        return -1;
    }

    /* Send message to Directory Server */
    if (sendto(fd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen) == -1) {

        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    /* Receive response from Directory Server */
    nread = recvfrom(fd, response, response_size - 1, 0, NULL, NULL);

    if (nread == -1) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    /* Add string terminator */
    response[nread] = '\0';

    freeaddrinfo(res);
    close(fd);

    return 0;
}