#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "peer_server.h"
#include "validation.h"

/* send() pode escrever apenas parte dos bytes pedidos. */
static int send_all(int fd, const void *data, size_t size)
{
    const char *bytes = data;
    size_t sent = 0;

    while (sent < size) {
        ssize_t n = send(fd, bytes + sent, size - sent, MSG_NOSIGNAL);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

int create_peer_listener(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        return -1;
    }

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((unsigned short)port);

    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(fd);
        return -1;
    }

    if (listen(fd, 5) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

/* Este processo continua a aceitar peers mesmo durante comandos do utilizador. */
void run_peer_server(int listener_fd, int stop_fd)
{
    signal(SIGCHLD, SIG_IGN);

    while (1) {
        fd_set ready;
        FD_ZERO(&ready);
        FD_SET(listener_fd, &ready);
        FD_SET(stop_fd, &ready);
        int largest = listener_fd > stop_fd ? listener_fd : stop_fd;

        if (select(largest + 1, &ready, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (FD_ISSET(stop_fd, &ready)) {
            break;
        }
        if (FD_ISSET(listener_fd, &ready)) {
            int peer_fd = accept(listener_fd, NULL, NULL);
            if (peer_fd < 0) {
                continue;
            }
            pid_t child = fork();
            if (child == 0) {
                close(listener_fd);
                close(stop_fd);
                handle_peer_request(peer_fd);
                close(peer_fd);
                _exit(0);
            }
            close(peer_fd);
        }
    }
    close(listener_fd);
    close(stop_fd);
}

void handle_peer_request(int peer_fd)
{
    char request[30];
    char command[4], filename[25], extra;
    char header[64], data[4096];
    struct timeval timeout = {5, 0};
    size_t used = 0;
    long file_size;
    FILE *file;

    if (setsockopt(peer_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
        setsockopt(peer_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0) {
        return;
    }

    while (used < sizeof(request) - 1) {
        ssize_t n = read(peer_fd, &request[used], 1);

        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            return;
        }
        if (request[used++] == '\n') {
            break;
        }
    }

    if (used == 0 || request[used - 1] != '\n') {
        send_all(peer_fd, "RGT ERR\n", 8);
        return;
    }
    request[used] = '\0';

    if (!validate_protocol_message(request, used) ||
        sscanf(request, "%3s %24s %c", command, filename, &extra) != 2 ||
        strcmp(command, "GET") != 0 || !validate_filename(filename)) {
        send_all(peer_fd, "RGT ERR\n", 8);
        return;
    }

    file_size = get_file_size(filename);
    if (file_size < 0 || (file = fopen(filename, "rb")) == NULL) {
        send_all(peer_fd, "RGT NOK\n", 8);
        return;
    }

    /* O cabecalho termina com espaco; os proximos file_size bytes sao dados. */
    int header_size = snprintf(header, sizeof(header), "RGT OK %ld ", file_size);
    if (send_all(peer_fd, header, (size_t)header_size) != 0) {
        fclose(file);
        return;
    }
    for (long remaining = file_size; remaining > 0;) {
        size_t chunk = remaining < (long)sizeof(data) ? (size_t)remaining : sizeof(data);
        size_t n = fread(data, 1, chunk, file);
        if (n == 0 || send_all(peer_fd, data, n) != 0) {
            fclose(file);
            return;
        }
        remaining -= (long)n;
    }
    send_all(peer_fd, "\n", 1);
    fclose(file);
}
