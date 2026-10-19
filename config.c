#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

/* strtol tambem aceita sinais e espacos; verificar primeiro todos os digitos. */
static int parse_port(const char *text, int *port)
{
    char *end;
    long value;
    if (*text == '\0')
        return 0;
    for (const char *p = text; *p; p++) {
        if (*p < '0' || *p > '9')
            return 0;
    }
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno != 0 || *end != '\0' || value < 1 || value > 65535)
        return 0;
    *port = (int)value;
    return 1;
}

int parse_arguments(int argc, char *argv[], AppConfig *config)
{
    int seen_m = 0, seen_n = 0, seen_p = 0;
    struct in_addr address;
    memset(config, 0, sizeof(*config));
    strcpy(config->ds_ip, DEFAULT_DS_IP);
    config->ds_port = DEFAULT_DS_PORT;

    for (int i = 1; i < argc; i += 2) {
        const char *option = argv[i];
        if (strcmp(option, "-m") != 0 && strcmp(option, "-n") != 0 &&
            strcmp(option, "-p") != 0) {
            printf("Error: unknown option %s\n", option);
            return 1;
        }
        if (i + 1 >= argc) {
            printf("Error: missing value after %s\n", option);
            return 1;
        }
        if (strcmp(option, "-n") == 0) {
            if (seen_n++ || strlen(argv[i + 1]) >= sizeof(config->ds_ip) ||
                inet_pton(AF_INET, argv[i + 1], &address) != 1) {
                printf("Error: -n requires a valid IPv4 address and cannot be repeated\n");
                return 1;
            }
            strcpy(config->ds_ip, argv[i + 1]);
        } else {
            int *seen = strcmp(option, "-m") == 0 ? &seen_m : &seen_p;
            int *port = strcmp(option, "-m") == 0 ? &config->peer_port : &config->ds_port;
            if ((*seen)++ || !parse_port(argv[i + 1], port)) {
                printf("Error: %s requires a decimal port between 1 and 65535 and cannot be repeated\n", option);
                return 1;
            }
        }
    }
    if (!seen_m) {
        printf("Error: missing -m option\n");
        return 1;
    }
    return 0;
}
