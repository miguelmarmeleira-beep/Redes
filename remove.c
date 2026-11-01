#include <stdio.h>
#include <string.h>

#include "remove.h"
#include "udp.h"
#include "validation.h"

int remove_resource(const AppConfig *config, const char *uid, const char *password,
                    const char *filename)
{
    char message[256], response[256], status[10];

    if (!validate_filename(filename)) {
        printf("invalid filename syntax\n");
        return -1;
    }

    snprintf(message, sizeof(message), "REM %s %s %s\n", uid, password, filename);
    int result = send_udp_request(config, message, response, sizeof(response));
    if (result != 0) {
        printf("%s\n", result == -2 ? "invalid response from Directory Server"
                          : "error communicating with Directory Server");
        return -1;
    }
    if (!parse_status_response(response, "RRM", status)) {
        printf("invalid response from Directory Server\n");
        return -1;
    }

    if (strcmp(status, "OK") == 0) {
        printf("successful removal\n");
        return 0;
    }
    if (strcmp(status, "NLG") == 0) {
        printf("user not logged in\n");
        return -1;
    }
    if (strcmp(status, "WRP") == 0) {
        printf("incorrect password\n");
        return -1;
    }
    if (strcmp(status, "NOK") == 0) {
        printf("resource not found\n");
        return -1;
    }
    if (strcmp(status, "UNR") == 0) {
        printf("unknown user\n");
        return -1;
    }
    if (strcmp(status, "ERR") == 0) {
        printf("unsuccessful removal\n");
        return -1;
    }
    printf("invalid response from Directory Server\n");
    return -1;
}
