#include <stdio.h>
#include <string.h>

#include "logout.h"
#include "udp.h"

int logout(const AppConfig *config,
          const char *uid,
          const char *password)
{
    char message[256], response[256], response_type[10], status[10];

    /* Build logout message */
    snprintf(message, sizeof(message), "LOU %s %s\n", uid, password);

    printf("Sending: %s", message);

    /* Send logout request and receive response */
    if (send_udp_request(config, message, response, sizeof(response)) != 0) {
        printf("Error communicating with Directory Server\n");
        return -1;
    }

    printf("Received: %s", response);

    /* Parse Directory Server response */
    if (sscanf(response, "%9s %9s", response_type, status) != 2) {
        printf("Invalid response from Directory Server\n");
        return -1;
    }

    /* Check response type */
    if (strcmp(response_type, "RLO") != 0) {
        printf("Unexpected response from Directory Server\n");
        return -1;
    }

    /* Check logout status */
    if (strcmp(status, "OK") == 0) {
        printf("Successful logout\n");
        return 0;
    }

    if (strcmp(status, "NLG") == 0) {
        printf("User not logged in\n");
        return -1;
    }

    if (strcmp(status, "UNR") == 0) {
        printf("Unknown user\n");
        return -1;
    }

    if (strcmp(status, "WRP") == 0) {
        printf("Password incorrect\n");
        return -1;
    }

    printf("Unknown logout status\n");
    return -1;
}