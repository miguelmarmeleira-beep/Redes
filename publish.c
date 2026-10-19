#include <stdio.h>
#include <string.h>

#include "publish.h"
#include "udp.h"
#include "validation.h"

int publish(const AppConfig *config,
            const char *uid,
            const char *password,
            const char *filename,
            const char *label)
{
    char message[512], response[256], status[10];
    long file_size;

    /* 1. Validar sintaxe do nome do ficheiro e da label */
    if (!validate_filename(filename) || !validate_label(label)) {
        printf("unsuccessful publication\n");
        return -1;
    }

    /* 2. Verificar se o ficheiro existe no diretorio local e obter tamanho */
    file_size = get_file_size(filename);
    if (file_size < 0) {
        /* Ficheiro nao existe (retornou -1) ou excede 10MB (retornou -2) */
        printf("unsuccessful publication\n");
        return -1;
    }

    /* 3. Construir a mensagem: PUB UID password filename Fsize label\n */
    snprintf(message, sizeof(message), "PUB %s %s %s %ld %s\n",
             uid, password, filename, file_size, label);

    /* 4. Enviar pedido UDP ao Directory Server e receber resposta */
    if (send_udp_request(config, message, response, sizeof(response)) != 0) {
        printf("unsuccessful publication\n");
        return -1;
    }

    /* 5. Fazer parse da resposta: esperado "RPB status" */
    if (!parse_status_response(response, "RPB", status)) {
        printf("unsuccessful publication\n");
        return -1;
    }

    /* 7. Tratar os possiveis status de acordo com o enunciado */
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

    if (strcmp(status, "NOK") == 0 || strcmp(status, "UNR") == 0) {
        printf("unsuccessful publication\n");
        return -1;
    }

    /* Caso receba ERR ou algo imprevisto */
    printf("unsuccessful publication\n");
    return -1;
}
