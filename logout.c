#include <stdio.h>
#include <string.h>

#include "logout.h"
#include "udp.h"
#include "validation.h"

int logout(const AppConfig *config,
          const char *uid,
          const char *password)
{
    char message[256], response[256], status[10];

    /* Build logout message */
    snprintf(message, sizeof(message), "LOU %s %s\n", uid, password);

    //printf("Sending: %s", message);

    /* Send logout request and receive response */
    if (send_udp_request(config, message, response, sizeof(response)) != 0) {
        printf("error communicating with Directory Server\n");
        return -1;
    }

    //printf("Received: %s", response);

    /* Parse Directory Server response */
    if (!parse_status_response(response, "RLO", status)) {
        printf("Invalid response from Directory Server\n");
        return -1;
    }

    /* Check logout status */
    if (strcmp(status, "OK") == 0) {
        printf("successful logout\n");
        return 0;
    }

    if (strcmp(status, "NLG") == 0) {
        printf("user not logged in\n");
        return -1;
    }

    if (strcmp(status, "UNR") == 0) {
        printf("unknown user\n");
        return -1;
    }

    if (strcmp(status, "WRP") == 0) {
        printf("password incorrect\n");
        return -1;
    }

    printf("unknown logout status\n");
    return -1;
}
