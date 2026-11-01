#include <stdio.h>
#include <string.h>

#include "login.h"
#include "udp.h"
#include "validation.h"

int login(const AppConfig *config, const char *uid, const char *password)
{
    char message[256], response[256], status[10];

    if (!validate_uid(uid) || !validate_password(password)) {
        printf("incorrect login attempt\n");
        return -1;
    }

    snprintf(message, sizeof(message), "LIN %s %s %d\n", uid, password, config->peer_port);
    int result = send_udp_request(config, message, response, sizeof(response));
    if (result != 0) {
        printf("%s\n", result == -2 ? "invalid response from Directory Server"
                          : "error communicating with Directory Server");
        return -1;
    }
    if (!parse_status_response(response, "RLI", status)) {
        printf("invalid response from Directory Server\n");
        return -1;
    }

    if (strcmp(status, "OK") == 0) {
        printf("successful login\n");
        return 0;
    }
    if (strcmp(status, "REG") == 0) {
        printf("new user registered\n");
        return 0;
    }
    if (strcmp(status, "NOK") == 0 || strcmp(status, "ERR") == 0) {
        printf("incorrect login attempt\n");
        return -1;
    }
    printf("invalid response from Directory Server\n");
    return -1;
}
