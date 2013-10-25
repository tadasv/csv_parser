#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <csv_parser.h>


typedef struct field_ {
    char data[32];
    int row;
    int col;
    struct field_ *next;
} field_t;


typedef struct csv_ {
    field_t *head;
    field_t *tail;
} csv_t;


field_t *new_field(const char *data, size_t length, int row, int col)
{
    field_t *field = malloc(sizeof(field_t));
    memset(field, 0, sizeof(field_t));
    field->row = row;
    field->col = col;
    strncpy(field->data, data, length);
    return field;
}


int field_cb(csv_parser_t *parser, const char *data, size_t length, int row, int col)
{
    csv_t *csv = parser->data;
    field_t *field;

    if (!csv->tail) {
        field = new_field(data, length, row, col);
        csv->tail = field;
        csv->head = field;
    } else {
        if (csv->tail->row == row && csv->tail->col == col) {
            strncat(csv->tail->data, data, length);
        } else {
            field = new_field(data, length, row, col);
            csv->tail->next = field;
            csv->tail = field;
        }
    }

    return 0;
}


int main(int argc, const char *argv[])
{
    csv_t csv;
    field_t *field;
    char buffer[64];
    ssize_t nread;
    csv_parser_t parser;
    csv_parser_settings_t settings;

    csv.head = NULL;
    csv.tail = NULL;

    settings.delimiter = ',';
    settings.field_cb = field_cb;

    csv_parser_init(&parser);
    parser.data = &csv;

    int fd = open("iris.csv", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    while ((nread = read(fd, buffer, sizeof(buffer))) > 0) {
        csv_parser_execute(&parser, &settings, buffer, nread);
    }

    close(fd);

    field = csv.head;
    while (field) {
        printf("row: %3d, col: %3d, data: %s\n", field->row, field->col, field->data);
        field_t *next = field->next;
        free(field);
        field = next;
    }

    return 0;
}
