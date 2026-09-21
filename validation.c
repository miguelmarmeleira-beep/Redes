#include <ctype.h>
#include <string.h>

#include "validation.h"

int validate_port (int port){
	
	if (port < 1 || port > 65535)
		return 0;
	
	return 1;
}

int validate_uid(const char *uid){
	if (strlen(uid) != 6){
		return 0;
	}
	for (int i = 0; i < 6; i++){
		if(!isdigit((unsigned char)uid[i]))
			return 0;
	}

	return 1;
}

int validate_password(const char *password){
	if (strlen(password) != 8)
		return 0;

	for (int i = 0; i < 8; i++){
		if(!isalnum((unsigned char)password[i]))
			return 0;
	}

	return 1;
}