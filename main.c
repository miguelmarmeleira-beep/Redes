#include <stdio.h>

#include "config.h"

int main(int argc, char *argv[])
{
    AppConfig config;

    if (parse_arguments(argc, argv, &config) != 0)
        return 1;

    printf("NetBoX started.\n");
    printf("Peer port: %d\n", config.peer_port);
	printf("DS IP: %s\n", config.ds_ip);
	printf("DS port: %d\n", config.ds_port);

    return 0;
}