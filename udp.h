#ifndef UDP_H
#define UDP_H

#include "config.h"

int send_udp_request(const AppConfig *config, const char *message, 
	char *response, int response_size);

#endif