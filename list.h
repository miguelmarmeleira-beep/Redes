#ifndef LIST_H
#define LIST_H

#include "config.h"

/* Pede ao Directory Server a lista de recursos publicados (via UDP)
   e mostra-a no ecra num formato legivel.
   Devolve 0 em caso de sucesso, ou -1 em caso de erro. */
int list_resources(const AppConfig *config);

#endif