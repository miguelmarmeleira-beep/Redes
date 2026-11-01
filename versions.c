#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "versions.h"
#include "tcp.h"
#include "validation.h"

int versions(const AppConfig *config, const char *filename)
{
    char message[128], *response;
    char *type, *status, *token;
    struct {
        char *uid, *label, *time, *availability;
        long size;
    } *entries = NULL;
    size_t capacity;
    int result;
    size_t count = 0;

    if (!validate_filename(filename)) {
        printf("invalid filename syntax\n");
        return -1;
    }
    snprintf(message, sizeof(message), "VRS %s\n", filename);
    result = send_tcp_request(config, message, &response);
    if (result != 0) {
        printf("%s\n", result == -2 ? "invalid response from Directory Server"
                          : "error communicating with Directory Server");
        return -1;
    }
    /* Cada registo completo ocupa pelo menos 18 bytes, incluindo separadores. */
    capacity = strlen(response) / 18;
    type = strtok(response, " \n");
    status = strtok(NULL, " \n");
    if (type == NULL || status == NULL || strcmp(type, "RVR") != 0)
        goto invalid;
    token = strtok(NULL, " \n");
    if (strcmp(status, "NOK") == 0 && token == NULL) {
        printf("no versions available for this resource\n");
        free(response);
        return 0;
    }
    if (strcmp(status, "OK") != 0)
        goto invalid;
    if (capacity == 0)
        goto invalid;
    entries = calloc(capacity, sizeof(*entries));
    if (entries == NULL) {
        printf("not enough memory to display versions\n");
        free(response);
        return -1;
    }
    while (token != NULL) {
        char *size, *label, *time, *availability;
        long value;
        if (count == capacity || !validate_uid(token))
            goto invalid;
        size = strtok(NULL, " \n");
        label = strtok(NULL, " \n");
        time = strtok(NULL, " \n");
        availability = strtok(NULL, " \n");
        if (size == NULL || label == NULL || time == NULL || availability == NULL ||
            !parse_file_size(size, &value) || !validate_label(label) ||
            (strcmp(availability, "AVL") != 0 && strcmp(availability, "NAV") != 0))
            goto invalid;
        /* O enunciado nao define o formato de publication_time; preservar o campo. */
        entries[count].uid = token;
        entries[count].size = value;
        entries[count].label = label;
        entries[count].time = time;
        entries[count].availability = availability;
        count++;
        token = strtok(NULL, " \n");
    }
    if (count == 0)
        goto invalid;
    printf("Available versions for '%s':\n", filename);
    printf("%-4s %-10s %-10s %-18s %-20s %-10s\n",
           "No.", "UID", "Size(B)", "Label", "Publication time", "Status");
    for (size_t i = 0; i < count; i++) {
        printf("%-4zu %-10s %-10ld %-18s %-20s %-10s\n", i + 1,
               entries[i].uid, entries[i].size, entries[i].label, entries[i].time,
               strcmp(entries[i].availability, "AVL") == 0 ? "online" : "offline");
    }
    free(entries);
    free(response);
    return 0;
invalid:
    printf("invalid response from Directory Server\n");
    free(entries);
    free(response);
    return -1;
}
