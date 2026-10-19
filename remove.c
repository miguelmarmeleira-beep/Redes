#include <stdio.h>
#include <string.h>

#include "remove.h"
#include "udp.h"
#include "validation.h"

int remove_resource(const AppConfig *config,
                    const char *uid,
                    const char *password,
                    const char *filename)
{
    char message[256], response[256], status[10];

    /* 1. Validar a sintaxe do nome do ficheiro */
    if (!validate_filename(filename)) {
        printf("resource not found\n");
        return -1;
    }

    /* 2. Construir a mensagem UDP: REM UID password filename\n */
    snprintf(message, sizeof(message), "REM %s %s %s\n", uid, password, filename);

    /* 3. Enviar o pedido ao Directory Server e receber a resposta */
    if (send_udp_request(config, message, response, sizeof(response)) != 0) {
        printf("resource not found\n");
        return -1;
    }

    /* 4. Fazer parse da resposta: esperado "RRM status" */
    if (!parse_status_response(response, "RRM", status)) {
        printf("resource not found\n");
        return -1;
    }

    /* 6. Tratar cada status possivel de acordo com o enunciado */
    if (strcmp(status, "OK") == 0) {
        printf("successful removal\n");
        return 0;
    }

    if (strcmp(status, "NOK") == 0) {
        printf("resource not found\n");
        return -1;
    }

    if (strcmp(status, "NLG") == 0) {
        printf("user not logged in\n");
        return -1;
    }

    if (strcmp(status, "WRP") == 0) {
        printf("incorrect password\n");
        return -1;
    }

    if (strcmp(status, "UNR") == 0) {
        printf("resource not found\n");
        return -1;
    }

    printf("resource not found\n");
    return -1;
}
