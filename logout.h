#ifndef LOGOUT_H
#define LOGOUT_H

#include "config.h"

/* 0: sucesso; 1: sessao inexistente; -1: falha, manter estado local. */
int logout(const AppConfig *config,
          const char *uid,
          const char *password);

#endif
