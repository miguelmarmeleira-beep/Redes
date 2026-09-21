#include "validation.h"

int validate_port (int port){
	
	if (port < 1 || port > 65535)
		return 0;
	
	return 1;
}