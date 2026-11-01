#ifndef UNREGISTER_H
#define UNREGISTER_H

#include "config.h"

/* 0: sucesso; 1: sessao inexistente; -1: falha, manter estado local. */
int unregister(const AppConfig *config,
          const char *uid,
          const char *password);

#endif
