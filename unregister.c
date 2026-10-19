#include <stdio.h>
#include <string.h>

#include "unregister.h"
#include "udp.h"
#include "validation.h"

int unregister(const AppConfig *config,
          const char *uid,
          const char *password)
{
    char message[256], response[256], status[10];

	/*`Build unregister message*/
    snprintf(message, sizeof(message), "UNR %s %s\n", uid, password);

    //printf("Sending: %s", message);

	/*send unregister request and receive response*/
    if (send_udp_request(config, message, response, sizeof(response)) != 0) {
        printf("incorrect unregister attempt\n");
        return -1;
    }

    //printf("received: %s", response);

    /* Parse Directory Server response */
    if (!parse_status_response(response, "RUR", status)) {
        printf("incorrect unregister attempt\n");
        return -1;
    }

    /* Check unregister status */
    if (strcmp(status, "OK") == 0) {
        printf("successful unregister\n");
        return 0;
    }

    if (strcmp(status, "NOK") == 0) {
		printf("user not logged in\n");
        return -1;
    }

	if(strcmp(status, "UNR") == 0){
		printf("unknown user\n");
		return -1;
	}

	if(strcmp(status, "WRP") == 0){
		printf("incorrect unregister attempt\n");
		return -1;
	}

    printf("incorrect unregister attempt\n");
    return -1;
}
