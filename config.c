#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "validation.h"

int parse_arguments(int argc, char *argv[], AppConfig *config)
{

	strcpy(config->ds_ip, DEFAULT_DS_IP);
	config->ds_port = DEFAULT_DS_PORT;

    if (argc < 3) {
        printf("Usage: ./user -m peerport\n");
        return 1;
    }

    if (strcmp(argv[1], "-m") != 0) {
        printf("Error: missing -m option\n");
        return 1;
    }

    config->peer_port = atoi(argv[2]);

	for(int i = 3; i < argc; i++){
		if (strcmp(argv[i], "-n") == 0){
			strcpy(config->ds_ip, argv[i+1]);
		}
		else if (strcmp(argv[i], "-p") == 0){
			config->ds_port = atoi(argv[i+1]);
		}
	}

	if (!validate_port(config->peer_port)){
		printf("Error: peer port must be between 1 and 65535\n");
		return 1;
	}

	if (!validate_port(config->ds_port)){
    	printf("Error: DS port must be between 1 and 65535\n");
    	return 1;
	}
	
    return 0;
}