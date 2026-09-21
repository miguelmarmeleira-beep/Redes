#ifndef	UNREGISTER_H
#define UNREGISTER_H

#include "config.h"

int unregister(const AppConfig *config,
          const char *uid,
          const char *password);

#endif