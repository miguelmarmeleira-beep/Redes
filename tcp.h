#ifndef TCP_H
#define TCP_H

#include "config.h"

/* Envia uma mensagem TCP para o Directory Server e exige uma resposta completa terminada em '\n'.
   Devolve 0 em caso de sucesso, ou -1 em caso de erro. */
int send_tcp_request(const AppConfig *config,
                     const char *message,
                     char *response,
                     int response_size);

#endif