#include <stdio.h>
#include <string.h>

#include "config.h"
#include "login.h"
#include "logout.h"
#include "unregister.h"

int main(int argc, char *argv[])
{
    AppConfig config;

    if (parse_arguments(argc, argv, &config) != 0)
        return 1;

    printf("NetBoX started.\n");

    char input[256], command[20], uid[20], password[50];

    int logged_in = 0;
    char current_uid[20] = "";
    char current_password[50] = "";

    while (1) {

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        if (sscanf(input, "%19s", command) != 1)
            continue;

        if (strcmp(command, "login") == 0) {

            if (sscanf(input, "%19s %19s %49s", command, uid, password) == 3) {

                if (login(&config, uid, password) == 0) {
                    logged_in = 1;
                    strcpy(current_uid, uid);
                    strcpy(current_password, password);
                }
            }
        }

        else if (strcmp(command, "logout") == 0) {

            if (!logged_in) {
                printf("User not logged in\n");
            }
            else if (logout(&config, current_uid, current_password) == 0) {
                logged_in = 0;
                current_uid[0] = '\0';
                current_password[0] = '\0';
            }
        }

		else if(strcmp(command, "unregister") == 0){
			if(!logged_in){
				printf("User not logged in\n");
			}
			else if(unregister(&config, current_uid, current_password) == 0){
				logged_in = 0;
                current_uid[0] = '\0';
                current_password[0] = '\0';
			}
		}

		else if(strcmp(command, "exit") == 0){
			if (logged_in){
				printf("Logout first\n");
			}
			else {
				return 0;
			}
		}
    }

    return 0;
}