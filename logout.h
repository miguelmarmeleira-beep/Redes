#ifndef LOGOUT_H
#define LOGOUT_H

#include "config.h"

int logout(const AppConfig *config,
          const char *uid,
          const char *password);

#endif