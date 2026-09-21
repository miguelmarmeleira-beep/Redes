#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_DS_IP "193.136.138.142"
#define DEFAULT_DS_PORT 59000

typedef struct {
    int peer_port;
	char ds_ip[64];
	int ds_port;
} AppConfig;

int parse_arguments(int argc, char *argv[], AppConfig *config);

#endif