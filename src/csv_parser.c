/**
 * Copyright (C) 2013 Tadas Vilkeliskis <vilkeliskis.t@gmail.com>
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <assert.h>
#include <stdlib.h>
#include <csv_parser.h>


void csv_parser_init(csv_parser_t *parser)
{
    assert(parser);
    parser->state = csvps_line_start;
    parser->row = -1;
    parser->col = -1;
    parser->nread = 0;
    parser->data = NULL;
}


size_t csv_parser_execute(csv_parser_t *parser,
                          const csv_parser_settings_t *settings,
                          const char *data,
                          size_t data_len)
{
    assert(parser);
    assert(settings);
    assert(data);

    const char *cursor = data;
    const char *field_value = NULL;
    const char *data_end = data + data_len;
    int r;
    parser->nread = 0;

    if (data_len < 1) {
        return 0;
    }

    while (cursor < data_end) {
        char ch = *cursor;
        switch (parser->state) {
            case csvps_line_start:
                field_value = NULL;
                parser->row += 1;
                parser->col = -1;
                if (ch == '\r' || ch == '\n') {
                    parser->state = csvps_line_end_begin;
                } else {
                    parser->state = csvps_field_start;
                }
                break;
            case csvps_field_start:
                parser->col += 1;
                if (ch == '"') {
                    parser->state = csvps_field_quoted_value;
                    cursor++;
                    parser->nread++;
                    field_value = cursor;
                } else {
                    parser->state = csvps_field_value;
                    field_value = cursor;
                }
                break;
            case csvps_field_value:
                if (ch == settings->delimiter) {
                    parser->state = csvps_field_end;
                } else if (ch == '\r' || ch == '\n') {
                    parser->state = csvps_line_end_begin;
                } else {
                    if (field_value == NULL) {
                        field_value = cursor;
                    }

                    cursor++;
                    if (cursor == data_end) {
                        if (settings->field_cb && field_value) {
                            r = settings->field_cb(parser,
                                                   field_value,
                                                   cursor - field_value,
                                                   parser->row,
                                                   parser->col);
                            if (r) {
                                parser->state = csvps_error;
                                parser->nread++;
                                return parser->nread;
                            }
                        }
                    }
                    parser->nread++;
                }
                break;
            case csvps_field_quoted_value:
                if (field_value == NULL) {
                    field_value = cursor;
                }
                if (ch == '"') {
                    parser->state = csvps_field_quoted_quote;
                    if (settings->field_cb && field_value && (cursor >= field_value)) {
                        r = settings->field_cb(parser,
                                               field_value,
                                               cursor - field_value,
                                               parser->row,
                                               parser->col);
                        if (r) {
                            parser->state = csvps_error;
                            return parser->nread;
                        }
                    }
                    field_value = NULL;
                    cursor++;
                    parser->nread++;
                } else {
                    cursor++;
                    if (cursor == data_end) {
                        if (settings->field_cb && field_value) {
                            r = settings->field_cb(parser,
                                                   field_value,
                                                   cursor - field_value,
                                                   parser->row,
                                                   parser->col);
                            if (r) {
                                parser->state = csvps_error;
                                parser->nread++;
                                return parser->nread;
                            }
                        }
                    }
                    parser->nread++;
                }
                break;
            case csvps_field_quoted_quote:
                if (ch == '"') {
                    if (settings->field_cb) {
                        r = settings->field_cb(parser, cursor, 1, parser->row, parser->col);
                        if (r) {
                            parser->state = csvps_error;
                            return parser->nread;
                        }
                    }
                    parser->state = csvps_field_quoted_value;
                    cursor++;
                    parser->nread++;
                    field_value = cursor;
                } else if (ch == settings->delimiter) {
                    parser->state = csvps_field_end;
                } else if (ch == '\r' || ch == '\n') {
                    parser->state = csvps_line_end_begin;
                } else {
                    parser->state = csvps_error;
                    return parser->nread;
                }
                break;
            case csvps_field_end:
                // callback
                if (settings->field_cb && field_value) {
                    r = settings->field_cb(parser,
                                           field_value,
                                           cursor - field_value,
                                           parser->row,
                                           parser->col);
                    if (r) {
                        parser->state = csvps_error;
                        return parser->nread;
                    }
                }

                parser->state = csvps_field_start;
                cursor++;
                parser->nread++;
                break;
            case csvps_line_end_begin:
                // callback
                if (settings->field_cb && field_value) {
                    r = settings->field_cb(parser,
                                           field_value,
                                           cursor - field_value,
                                           parser->row,
                                           parser->col);
                    if (r) {
                        parser->state = csvps_error;
                        return parser->nread;
                    }
                }
                if (ch == '\r') {
                    parser->state = csvps_line_end;
                } else {
                    parser->state = csvps_line_start;
                }
                cursor++;
                parser->nread++;
                break;
            case csvps_line_end:
                if (ch == '\n') {
                    cursor++;
                    parser->nread++;
                }
                parser->state = csvps_line_start;
                break;
            case csvps_error:
                return parser->nread;
                break;
            default:
                assert(0 && "invalid parser state");
                break;
        }
    }

    return parser->nread;
}

int csv_parser_finish(csv_parser_t *parser, const csv_parser_settings_t *settings)
{
    int r = 0;
    if (parser->state == csvps_field_start) {
        if (settings->field_cb) {
            parser->col += 1;
            r = settings->field_cb(parser, "", 0, parser->row, parser->col);
            if (r) {
                parser->state = csvps_error;
            }
        }
    }
    if (parser->state != csvps_error) {
        parser->state = csvps_line_start;
    }
    return r;
}
