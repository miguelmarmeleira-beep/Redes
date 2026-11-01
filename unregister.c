#include <stdio.h>
#include <string.h>

#include "unregister.h"
#include "udp.h"
#include "validation.h"

int unregister(const AppConfig *config, const char *uid, const char *password)
{
    char message[256], response[256], status[10];

    snprintf(message, sizeof(message), "UNR %s %s\n", uid, password);
    int result = send_udp_request(config, message, response, sizeof(response));
    if (result != 0) {
        printf("%s\n", result == -2 ? "invalid response from Directory Server"
                          : "error communicating with Directory Server");
        return -1;
    }
    if (!parse_status_response(response, "RUR", status)) {
        printf("invalid response from Directory Server\n");
        return -1;
    }

    if (strcmp(status, "OK") == 0) {
        printf("successful unregister\n");
        return 0;
    }
    if (strcmp(status, "NOK") == 0) {
        printf("user not logged in\n");
        return 1;
    }
    if (strcmp(status, "UNR") == 0) {
        printf("unknown user\n");
        return 1;
    }
    if (strcmp(status, "WRP") == 0 || strcmp(status, "ERR") == 0) {
        printf("incorrect unregister attempt\n");
        return -1;
    }
    printf("invalid response from Directory Server\n");
    return -1;
}
