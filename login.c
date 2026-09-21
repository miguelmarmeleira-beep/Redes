#include <stdio.h>
#include <string.h>

#include "login.h"
#include "udp.h"
#include "validation.h"

int login(const AppConfig *config,
          const char *uid,
          const char *password)
{
    char message[256], response[256],response_type[10], status[10];

	if (!validate_uid(uid)) {
        printf("Incorrect login attempt\n");
        return -1;
    }

    if (!validate_password(password)) {
        printf("Incorrect login attempt\n");
        return -1;
    }

	/*`Build login message*/
    snprintf(message, sizeof(message), "LIN %s %s %d\n", uid, password, config->peer_port);

    printf("Sending: %s", message);

	/*send login request and receive response*/
    if (send_udp_request(config, message, response, sizeof(response)) != 0) {
        printf("Incorret login attempt\n");
        return -1;
    }

    printf("Received: %s", response);

    /* Parse Directory Server response */
    if (sscanf(response, "%9s %9s", response_type, status) != 2) {
        printf("Incorret login attempt\n");
        return -1;
    }

    /* Check response type */
    if (strcmp(response_type, "RLI") != 0) {
        printf("Incorret login attempt\n");
        return -1;
    }

    /* Check login status */
    if (strcmp(status, "OK") == 0) {
        printf("Sucessful login\n");
        return 0;
    }

    if (strcmp(status, "NOK") == 0) {
		printf("Incorret login attempt\n");
        return -1;
    }

	if(strcmp(status, "REG") == 0){
		printf("New user registered\n");
		return 0;
	}

    printf("Incorret login attempt\n");
    return -1;
}