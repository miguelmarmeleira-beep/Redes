#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "login.h"
#include "logout.h"
#include "unregister.h"
#include "publish.h"
#include "remove.h"
#include "list.h"
#include "versions.h"
#include "where.h"
#include "validation.h"
#include "peer_server.h"
#include "get.h"

int main(int argc, char *argv[])
{
    AppConfig config;
    char input[256];
    int logged_in = 0;
    char current_uid[7] = "", current_password[9] = "";

    if (parse_arguments(argc, argv, &config) != 0) {
        return 1;
    }

    int listener_fd = create_peer_listener(config.peer_port);
    if (listener_fd < 0) {
        printf("could not open peer port\n");
        return 1;
    }
    int stop_pipe[2];
    if (pipe(stop_pipe) != 0) {
        close(listener_fd);
        printf("could not prepare peer server\n");
        return 1;
    }
    pid_t server_pid = fork();
    if (server_pid < 0) {
        close(stop_pipe[0]);
        close(stop_pipe[1]);
        close(listener_fd);
        printf("could not prepare peer server\n");
        return 1;
    }
    if (server_pid == 0) {
        close(stop_pipe[1]);
        run_peer_server(listener_fd, stop_pipe[0]);
        _exit(0);
    }
    close(stop_pipe[0]);
    close(listener_fd);
    printf("NetBoX started.\n");

    while (fgets(input, sizeof(input), stdin) != NULL) {

        char *args[4], *token;
        int count = 0;

        /* Descartar a linha inteira para nao executar o resto como comandos. */
        if (strchr(input, '\n') == NULL && strlen(input) == sizeof(input) - 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            printf("command too long\n");
            continue;
        }
        token = strtok(input, " \t\r\n");
        while (token != NULL && count < 4) {
            args[count++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        if (count == 0) {
            continue;
        }

        if (strcmp(args[0], "login") == 0) {
            if (count != 3) {
                printf("usage: login UID password\n");
            } else if (logged_in) {
                printf("Logout first\n");
            } else if (login(&config, args[1], args[2]) == 0) {
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
            if (count != 3) {
                printf("usage: publish filename label\n");
            } else if (!logged_in) {
                printf("user not logged in\n");
            } else {
                publish(&config, current_uid, current_password, args[1], args[2]);
            }

        } else if (strcmp(args[0], "remove") == 0) {
            if (count != 2) {
                printf("usage: remove filename\n");
            } else if (!logged_in) {
                printf("user not logged in\n");
            } else {
                remove_resource(&config, current_uid, current_password, args[1]);
            }

        } else if (strcmp(args[0], "list") == 0) {
            if (count != 1) {
                printf("usage: list\n");
            } else {
                list_resources(&config);
            }

        } else if (strcmp(args[0], "versions") == 0) {
            if (count != 2) {
                printf("usage: versions filename\n");
            } else {
                versions(&config, args[1]);
            }

        } else if (strcmp(args[0], "where") == 0) {
            if (count != 3) {
                printf("usage: where filename UID\n");
            } else if (!validate_filename(args[1]) || !validate_uid(args[2])) {
                printf("invalid filename or UID syntax\n");
            } else {
                char ip[16];
                int port;
                if (where_resource(&config, args[1], args[2], ip, &port) == 0) {
                    printf("peer available at %s:%d\n", ip, port);
                }
            }

        } else if (strcmp(args[0], "get") == 0) {
            if (count != 2) {
                printf("usage: get filename\n");
            } else {
                get_resource(&config, args[1]);
            }

        } else if (strcmp(args[0], "exit") == 0) {
            if (count != 1) {
                printf("usage: exit\n");
            } else if (logged_in) {
                printf("Logout first\n");
            } else {
                break;
            }
        } else {
            printf("unknown command: %s\n", args[0]);
        }
    }
    close(stop_pipe[1]);
    waitpid(server_pid, NULL, 0);
    return 0;
}
