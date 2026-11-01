#ifndef UDP_H
#define UDP_H

#include "config.h"

/* 0: sucesso; -1: erro de comunicacao; -2: resposta invalida. */
int send_udp_request(const AppConfig *config, const char *message, 
    char *response, int response_size);

#endif
