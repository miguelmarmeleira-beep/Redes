#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "get.h"
#include "tcp.h"
#include "validation.h"
#include "where.h"

/* Escolher a primeira versao online na resposta do DS. */
static int choose_online_uid(const AppConfig *config, const char *filename, char uid[7])
{
    char message[32], *response, *type, *status, *token;
    int count = 0, valid = 1;

    snprintf(message, sizeof(message), "VRS %s\n", filename);
    int result = send_tcp_request(config, message, &response);
    if (result != 0) {
        printf("%s\n", result == -2 ? "invalid response from Directory Server"
                          : "error communicating with Directory Server");
        return -1;
    }

    uid[0] = '\0';
    type = strtok(response, " \n");
    status = strtok(NULL, " \n");
    token = strtok(NULL, " \n");
    if (type == NULL || status == NULL || strcmp(type, "RVR") != 0) {
        valid = 0;
    } else if (strcmp(status, "NOK") == 0 && token == NULL) {
        free(response);
        printf("no versions available for this resource\n");
        return 1;
    } else if (strcmp(status, "OK") != 0) {
        valid = 0;
    }

    while (valid && token != NULL) {
        char *size = strtok(NULL, " \n");
        char *label = strtok(NULL, " \n");
        char *time = strtok(NULL, " \n");
        char *availability = strtok(NULL, " \n");
        long value;

        if (!validate_uid(token) || size == NULL || label == NULL || time == NULL ||
            availability == NULL || !parse_file_size(size, &value) ||
            !validate_label(label) ||
            (strcmp(availability, "AVL") != 0 && strcmp(availability, "NAV") != 0)) {
            valid = 0;
            break;
        }
        if (uid[0] == '\0' && strcmp(availability, "AVL") == 0) {
            strcpy(uid, token);
        }
        count++;
        token = strtok(NULL, " \n");
    }
    free(response);
    if (!valid || count == 0) {
        printf("invalid response from Directory Server\n");
        return -1;
    }
    if (uid[0] == '\0') {
        printf("no online peers for this resource\n");
        return 1;
    }
    return 0;
}

static int send_request(int fd, const char *message)
{
    size_t sent = 0, length = strlen(message);
    while (sent < length) {
        ssize_t n = send(fd, message + sent, length - sent, MSG_NOSIGNAL);
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

/* Ler um byte para separar o cabecalho de texto dos dados binarios. */
static int read_byte(int fd, char *byte)
{
    ssize_t n = read(fd, byte, 1);
    while (n < 0 && errno == EINTR) {
        n = read(fd, byte, 1);
    }
    return n == 1 ? 0 : -1;
}

static int receive_file(int fd, const char *filename)
{
    char header[32], data[4096];
    char temporary[] = ".netbox-download-XXXXXX";
    size_t used = 0;
    int spaces = 0;
    long file_size;

    while (used < sizeof(header) - 1) {
        if (read_byte(fd, &header[used]) != 0) {
            printf("error receiving file from peer\n");
            return -1;
        }
        if (header[used] == ' ') {
            spaces++;
        }
        if (header[used++] == '\n' || spaces == 3) {
            break;
        }
    }
    header[used] = '\0';
    if (strcmp(header, "RGT NOK\n") == 0) {
        printf("file not available from peer\n");
        return 1;
    }
    if (strcmp(header, "RGT ERR\n") == 0) {
        printf("peer rejected the request\n");
        return -1;
    }
    if (used < 9 || strncmp(header, "RGT OK ", 7) != 0 ||
        header[used - 1] != ' ') {
        printf("invalid response from peer\n");
        return -1;
    }
    header[used - 1] = '\0';
    if (!parse_file_size(header + 7, &file_size)) {
        printf("invalid response from peer\n");
        return -1;
    }

    int temporary_fd = mkstemp(temporary);
    if (temporary_fd < 0) {
        printf("error creating local file\n");
        return -1;
    }
    FILE *file = fdopen(temporary_fd, "wb");
    if (file == NULL) {
        close(temporary_fd);
        remove(temporary);
        printf("error creating local file\n");
        return -1;
    }

    int complete = 1;
    for (long remaining = file_size; remaining > 0;) {
        size_t wanted = remaining < (long)sizeof(data) ? (size_t)remaining : sizeof(data);
        ssize_t n = read(fd, data, wanted);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n <= 0 || fwrite(data, 1, (size_t)n, file) != (size_t)n) {
            complete = 0;
            break;
        }
        remaining -= n;
    }
    char end;
    if (complete && (read_byte(fd, &end) != 0 || end != '\n')) {
        complete = 0;
    }
    if (fclose(file) != 0) {
        complete = 0;
    }
    if (complete && rename(temporary, filename) == 0) {
        printf("received './%s' (%ld bytes)\n", filename, file_size);
        return 0;
    }
    remove(temporary);
    printf("error receiving or storing file\n");
    return -1;
}

int get_resource(const AppConfig *config, const char *filename)
{
    char uid[7], ip[16], message[32];
    int port;

    if (!validate_filename(filename)) {
        printf("invalid filename syntax\n");
        return -1;
    }
    if (choose_online_uid(config, filename, uid) != 0) {
        return -1;
    }
    if (where_resource(config, filename, uid, ip, &port) != 0) {
        return -1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("error connecting to peer\n");
        return -1;
    }
    struct timeval timeout = {5, 0};
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, ip, &address.sin_addr) != 1 ||
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0 ||
        connect(fd, (struct sockaddr *)&address, sizeof(address)) != 0) {
        close(fd);
        printf("error connecting to peer\n");
        return -1;
    }

    snprintf(message, sizeof(message), "GET %s\n", filename);
    if (send_request(fd, message) != 0) {
        close(fd);
        printf("error requesting file from peer\n");
        return -1;
    }
    int result = receive_file(fd, filename);
    close(fd);
    return result == 0 ? 0 : -1;
}
