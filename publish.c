#include <stdio.h>
#include <string.h>

#include "publish.h"
#include "udp.h"
#include "validation.h"

int publish(const AppConfig *config, const char *uid, const char *password,
            const char *filename, const char *label)
{
    char message[256], response[256], status[10];

    if (!validate_filename(filename) || !validate_label(label)) {
        printf("invalid filename or label syntax\n");
        return -1;
    }

    long file_size = get_file_size(filename);
    if (file_size < 0) {
        printf("unsuccessful publication: file unavailable or larger than 10 MB\n");
        return -1;
    }

    snprintf(message, sizeof(message), "PUB %s %s %s %ld %s\n",
             uid, password, filename, file_size, label);
    int result = send_udp_request(config, message, response, sizeof(response));
    if (result != 0) {
        printf("%s\n", result == -2 ? "invalid response from Directory Server"
                          : "error communicating with Directory Server");
        return -1;
    }
    if (!parse_status_response(response, "RPB", status)) {
        printf("invalid response from Directory Server\n");
        return -1;
    }

    if (strcmp(status, "OK") == 0) {
        printf("successful publication\n");
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
    if (strcmp(status, "NOK") == 0 || strcmp(status, "UNR") == 0 ||
        strcmp(status, "ERR") == 0) {
        printf("unsuccessful publication\n");
        return -1;
    }
    printf("invalid response from Directory Server\n");
    return -1;
}
