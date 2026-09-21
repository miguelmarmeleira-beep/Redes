#ifndef LOGIN_H
#define LOGIN_H

#include "config.h"

int login(const AppConfig *config,
          const char *uid,
          const char *password);

#endif