#ifndef WHERE_H
#define WHERE_H

#include "config.h"

/* 0: peer encontrado; 1: indisponivel; -1: erro.
   Em caso de sucesso, ip e port recebem a localizacao do peer. */
int where_resource(const AppConfig *config, const char *filename, const char *uid,
                   char ip[16], int *port);

#endif
