#include <stdio.h>
#include <string.h>

#include "config.h"
#include "login.h"
#include "logout.h"
#include "unregister.h"
#include "publish.h"
#include "remove.h"
#include "list.h"
#include "versions.h"

int main(int argc, char *argv[])
{
    AppConfig config;
    char input[256];
    int logged_in = 0;
    char current_uid[7] = "", current_password[9] = "";

    if (parse_arguments(argc, argv, &config) != 0)
        return 1;
    printf("NetBoX started.\n");

    while (fgets(input, sizeof(input), stdin) != NULL) {
        char *args[4], *token;
        int count = 0;

        /* Descartar a linha inteira para nao executar o resto como comandos. */
        if (strchr(input, '\n') == NULL && strlen(input) == sizeof(input) - 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("command too long\n");
            continue;
        }
        token = strtok(input, " \t\r\n");
        while (token != NULL && count < 4) {
            args[count++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        if (count == 0)
            continue;

        if (strcmp(args[0], "login") == 0) {
            if (count != 3)
                printf("usage: login UID password\n");
            else if (logged_in)
                printf("Logout first\n");
            else if (login(&config, args[1], args[2]) == 0) {
                logged_in = 1;
                strcpy(current_uid, args[1]);
                strcpy(current_password, args[2]);
            }

        } else if (strcmp(args[0], "logout") == 0 || strcmp(args[0], "unregister") == 0) {
            if (count != 1) {
                printf("usage: %s\n", args[0]);
            } else if (!logged_in) {
                printf("user not logged in\n");
            } else {
                int result = strcmp(args[0], "logout") == 0
                    ? logout(&config, current_uid, current_password)
                    : unregister(&config, current_uid, current_password);
                /* 1: o DS confirmou que a sessao ja nao existe. */
                if (result >= 0) {
                    logged_in = 0;
                    memset(current_uid, 0, sizeof(current_uid));
                    memset(current_password, 0, sizeof(current_password));
                }
            }

        } else if (strcmp(args[0], "publish") == 0) {
            if (count != 3)
                printf("usage: publish filename label\n");
            else if (!logged_in)
                printf("user not logged in\n");
            else
                publish(&config, current_uid, current_password, args[1], args[2]);

        } else if (strcmp(args[0], "remove") == 0) {
            if (count != 2)
                printf("usage: remove filename\n");
            else if (!logged_in)
                printf("user not logged in\n");
            else
                remove_resource(&config, current_uid, current_password, args[1]);

        } else if (strcmp(args[0], "list") == 0) {
            if (count != 1)
                printf("usage: list\n");
            else
                list_resources(&config);

        } else if (strcmp(args[0], "versions") == 0) {
            if (count != 2)
                printf("usage: versions filename\n");
            else
                versions(&config, args[1]);

        } else if (strcmp(args[0], "exit") == 0) {
            if (count != 1)
                printf("usage: exit\n");
            else if (logged_in)
                printf("Logout first\n");
            else
                return 0;
        } else {
            printf("unknown command: %s\n", args[0]);
        }
    }
    return 0;
}
