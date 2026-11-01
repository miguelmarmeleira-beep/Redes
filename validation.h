#ifndef VAL_H
#define VAL_H

#include <stddef.h>

int validate_uid(const char *uid);

int validate_password(const char *password);

int validate_filename(const char *filename);

int validate_label(const char *label);

long get_file_size(const char *filename);

/* Validar a mensagem completa, incluindo espacos e terminador. */
int validate_protocol_message(const char *message, size_t size);

/* Resposta simples: exatamente dois campos; status precisa de 10 bytes. */
int parse_status_response(const char *response, const char *expected, char *status);

int parse_file_size(const char *text, long *size);

#endif
