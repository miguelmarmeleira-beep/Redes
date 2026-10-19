#ifndef VERSIONS_H
#define VERSIONS_H

#include "config.h"

/* Envia o pedido 'versions' ao DS via TCP e mostra a lista formatada.
   Devolve 0 em caso de sucesso, ou -1 em caso de erro. */
int versions(const AppConfig *config, const char *filename);

#endif