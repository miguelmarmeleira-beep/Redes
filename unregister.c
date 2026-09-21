#include <stdio.h>
#include <string.h>

#include "unregister.h"
#include "udp.h"

int unregister(const AppConfig *config,
          const char *uid,
          const char *password)
{
    char message[256], response[256],response_type[10], status[10];

	/*`Build unregister message*/
    snprintf(message, sizeof(message), "UNR %s %s\n", uid, password);

    printf("Sending: %s", message);

	/*send unregister request and receive response*/
    if (send_udp_request(config, message, response, sizeof(response)) != 0) {
        printf("ERR\n");
        return -1;
    }

    printf("Received: %s", response);

    /* Parse Directory Server response */
    if (sscanf(response, "%9s %9s", response_type, status) != 2) {
        printf("ERR\n");
        return -1;
    }

    /* Check response type */
    if (strcmp(response_type, "RUR") != 0) {
        printf("ERR\n");
        return -1;
    }

    /* Check unregister status */
    if (strcmp(status, "OK") == 0) {
        printf("Successful unregister\n");
        return 0;
    }

    if (strcmp(status, "NOK") == 0) {
		printf("User not logged in\n");
        return -1;
    }

	if(strcmp(status, "UNR") == 0){
		printf("Unknown user\n");
		return -1;
	}

	if(strcmp(status, "WRP") == 0){
		printf("Incorrect unregister attempt\n");
		return -1;
	}

    printf("Incorrect unregister attempt\n");
    return -1;
}