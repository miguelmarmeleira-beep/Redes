#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "validation.h"

int validate_uid(const char *uid)
{
    if (strlen(uid) != 6)
        return 0;
    for (int i = 0; i < 6; i++) {
        if (!isdigit((unsigned char)uid[i]))
            return 0;
    }
    return 1;
}

int validate_password(const char *password)
{
    if (strlen(password) != 8)
        return 0;
    for (int i = 0; i < 8; i++) {
        if (!isalnum((unsigned char)password[i]))
            return 0;
    }
    return 1;
}

/* Valida se o nome cumpre: ate 24 carateres no total, base com alfanumericos, '-' ou '_',
   seguido de um ponto '.' e exatamente 3 carateres alfanumericos de extensao */
int validate_filename(const char *filename)
{
    size_t len = strlen(filename);
    if (len < 5 || len > 24)
        return 0;

    /* A extensão tem de começar no índice len - 4 com um ponto */
    if (filename[len - 4] != '.')
        return 0;

    /* Verifica os 3 caracteres da extensão */
    for (size_t i = len - 3; i < len; i++) {
        if (!isalnum((unsigned char)filename[i]))
            return 0;
    }

    /* Verifica os caracteres da base (antes do ponto) */
    for (size_t i = 0; i < len - 4; i++) {
        char c = filename[i];
        if (!isalnum((unsigned char)c) && c != '-' && c != '_')
            return 0;
    }

    return 1;
}

/* Valida a label: 1 a 20 carateres alfanumericos, '-' ou '_' */
int validate_label(const char *label)
{
    size_t len = strlen(label);
    if (len < 1 || len > 20)
        return 0;

    for (size_t i = 0; i < len; i++) {
        char c = label[i];
        if (!isalnum((unsigned char)c) && c != '-' && c != '_')
            return 0;
    }

    return 1;
}

/* Verifica se o ficheiro existe localmente e devolve o tamanho em bytes.
   Devolve -1 se o ficheiro nao existir ou -2 se exceder 10MB (10000000 bytes) */
long get_file_size(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size < 0)
        return -1; /* Nao existe */

    if (st.st_size > 10000000)
        return -2; /* Excede 10 MB */

    return (long)st.st_size;
}

int validate_protocol_message(const char *message, size_t size)
{
    if (size < 2 || message[size - 1] != '\n' || message[0] == ' ')
        return 0;
    for (size_t i = 0; i < size - 1; i++) {
        unsigned char c = (unsigned char)message[i];
        if (c < 32 || c > 126)
            return 0;
        if (c == ' ' && (i == size - 2 || message[i + 1] == ' '))
            return 0;
    }
    return 1;
}

int parse_status_response(const char *response, const char *expected, char *status)
{
    char type[10], extra;
    return sscanf(response, "%9s %9s %c", type, status, &extra) == 2 &&
           strcmp(type, expected) == 0;
}

int parse_file_size(const char *text, long *size)
{
    size_t len = strlen(text);
    long value = 0;
    if (len == 0 || len > 8)
        return 0;
    for (size_t i = 0; i < len; i++) {
        if (text[i] < '0' || text[i] > '9')
            return 0;
        value = value * 10 + text[i] - '0';
    }
    if (value > 10000000)
        return 0;
    *size = value;
    return 1;
}
