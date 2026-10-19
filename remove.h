#ifndef REMOVE_H
#define REMOVE_H

#include "config.h"

/* Envia o pedido de remocao de um ficheiro publicado ao Directory Server via UDP.
   Devolve 0 em caso de sucesso, ou -1 em caso de erro. */
int remove_resource(const AppConfig *config,
                    const char *uid,
                    const char *password,
                    const char *filename);

#endif