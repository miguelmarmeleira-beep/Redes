#ifndef TCP_H
#define TCP_H

#include "config.h"

/* Recebe uma resposta terminada em '\n', alocada dinamicamente.
   O chamador liberta *response com free() em caso de sucesso.
   0: sucesso; -1: erro de comunicacao/memoria; -2: resposta invalida.
   Em caso de erro, *response fica NULL. */
int send_tcp_request(const AppConfig *config, const char *message, char **response);

#endif
