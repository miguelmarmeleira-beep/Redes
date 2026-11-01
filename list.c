#include <stdio.h>
#include <string.h>

#include "list.h"
#include "udp.h"
#include "validation.h"

int list_resources(const AppConfig *config)
{
    char response[2048];
    char *type, *status, *token;
    char *filenames[50];
    int count = 0;

    int result = send_udp_request(config, "LST\n", response, sizeof(response));
    if (result != 0) {
        printf("%s\n", result == -2 ? "invalid response from Directory Server"
                          : "error communicating with Directory Server");
        return -1;
    }
    type = strtok(response, " \n");
    status = strtok(NULL, " \n");
    if (type == NULL || status == NULL || strcmp(type, "RLS") != 0)
        goto invalid;
    token = strtok(NULL, " \n");
    if (strcmp(status, "NOK") == 0 && token == NULL) {
        printf("no resources currently available\n");
        return 0;
    }
    if (strcmp(status, "OK") != 0)
        goto invalid;
    /* Validar a lista inteira antes de apresentar qualquer resultado. */
    while (token != NULL) {
        if (count == 50 || !validate_filename(token))
            goto invalid;
        for (int i = 0; i < count; i++) {
            if (strcmp(filenames[i], token) == 0)
                goto invalid;
        }
        filenames[count++] = token;
        token = strtok(NULL, " \n");
    }
    if (count == 0)
        goto invalid;
    printf("Available resources:\n");
    for (int i = 0; i < count; i++)
        printf("  %d. %s\n", i + 1, filenames[i]);
    return 0;
invalid:
    printf("invalid response from Directory Server\n");
    return -1;
}
