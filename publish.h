#ifndef PUBLISH_H
#define PUBLISH_H

#include "config.h"

/* Envia o pedido de publicacao de um ficheiro ao Directory Server via UDP.
   Devolve 0 em caso de sucesso, ou -1 em caso de erro. */
int publish(const AppConfig *config,
            const char *uid,
            const char *password,
            const char *filename,
            const char *label);

#endif