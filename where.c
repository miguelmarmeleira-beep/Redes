#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "where.h"
#include "udp.h"

int where_resource(const AppConfig *config, const char *filename, const char *uid,
                   char ip[16], int *port)
{
    char message[64], response[256];
    char type[10], status[10], address[16], port_text[6], extra;
    struct in_addr parsed_address;
    long parsed_port;
    int fields, result;

    snprintf(message, sizeof(message), "WHR %s %s\n", filename, uid);
    result = send_udp_request(config, message, response, sizeof(response));
    if (result != 0) {
        printf("%s\n", result == -2 ? "invalid response from Directory Server"
                          : "error communicating with Directory Server");
        return -1;
    }

    fields = sscanf(response, "%9s %9s %15s %5s %c", type, status, address, port_text, &extra);
    if (fields == 2 && strcmp(type, "RWH") == 0 && strcmp(status, "NOK") == 0) {
        printf("resource not available from this user\n");
        return 1;
    }
    if (fields == 2 && strcmp(type, "RWH") == 0 && strcmp(status, "ERR") == 0) {
        printf("request rejected by Directory Server\n");
        return -1;
    }
    if (fields != 4 || strcmp(type, "RWH") != 0 || strcmp(status, "OK") != 0 ||
        inet_pton(AF_INET, address, &parsed_address) != 1 ||
        *port_text == '\0' || strspn(port_text, "0123456789") != strlen(port_text)) {
        printf("invalid response from Directory Server\n");
        return -1;
    }
    parsed_port = strtol(port_text, NULL, 10);
    if (parsed_port < 1 || parsed_port > 65535) {
        printf("invalid response from Directory Server\n");
        return -1;
    }

    strcpy(ip, address);
    *port = (int)parsed_port;
    return 0;
}
