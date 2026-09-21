#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "validation.h"

int parse_arguments(int argc, char *argv[], AppConfig *config)
{
    int peer_port_found = 0;

    strcpy(config->ds_ip, DEFAULT_DS_IP);
    config->ds_port = DEFAULT_DS_PORT;

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-m") == 0) {

            if (i + 1 >= argc) {
                printf("Error: missing peer port after -m\n");
                return 1;
            }

            config->peer_port = atoi(argv[i + 1]);
            peer_port_found = 1;
            i++;
        }

        else if (strcmp(argv[i], "-n") == 0) {

            if (i + 1 >= argc) {
                printf("Error: missing DS IP after -n\n");
                return 1;
            }

            strcpy(config->ds_ip, argv[i + 1]);
            i++;
        }

        else if (strcmp(argv[i], "-p") == 0) {

            if (i + 1 >= argc) {
                printf("Error: missing DS port after -p\n");
                return 1;
            }

            config->ds_port = atoi(argv[i + 1]);
            i++;
        }
    }

    if (!peer_port_found) {
        printf("Error: missing -m option\n");
        return 1;
    }

    if (!validate_port(config->peer_port)) {
        printf("Error: peer port must be between 1 and 65535\n");
        return 1;
    }

    if (!validate_port(config->ds_port)) {
        printf("Error: DS port must be between 1 and 65535\n");
        return 1;
    }

    return 0;
}