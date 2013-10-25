#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <csv_parser.h>


struct test_results {
    int test_case_number;
    int *results;
};


// 5 items per row, 3 rows per test case: 15 * 6 slots
static int expected_test_results[] = {
    // ""
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    // ", "
    0, 1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    // ","
    0, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    // "\n"
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    // "\r\n",
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    // "a,b,,c,"
    1, 2, 0, 1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    // "a,b,,c, "
    1, 2, 0, 1, 1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    // "a,b,\na,b,"
    1, 2, 0, -1, -1,
    1, 1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    // " bc "
    4, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
};


static const int results_size = sizeof(expected_test_results);
static const int result_slots = sizeof(expected_test_results)/sizeof(int);


static const char *test_data[] = {
    "",
    ", ",
    ",",
    "\n",
    "\r\n",
    "a,bb,,c,",
    "a,bb,,c, ",
    "a,bb,\na,b,",
    " bc ",
};


static int test_data_end_states[] = {
    csvps_line_start,
    csvps_field_value,
    csvps_field_start,
    csvps_line_end,
    csvps_line_end,
    csvps_field_start,
    csvps_field_value,
    csvps_field_start,
    csvps_field_value,
};

static int num_test_cases = sizeof(test_data)/(sizeof(char*));


void init_test_results(int *data, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        data[i] = -1;
    }
}


void print_results(int *data, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        if (i != 0 && i % 5 == 0) {
            printf("\n");
        }
        printf("%4d ", data[i]);
    }

    printf("\n");
}


int field_cb(csv_parser_t *parser, const char *data, size_t length, int row, int col)
{
    struct test_results *results = parser->data;
    int slot = results->test_case_number * 15 + row * 5 + col;

    if (results->results[slot] == -1) {
        results->results[slot] = length;
    } else {
        results->results[slot] += length;
    }
    return 0;
}


START_TEST(test_parser_full)
{
    csv_parser_t parser;
    csv_parser_settings_t settings;
    struct test_results results;
    int i;

    settings.delimiter = ',';
    settings.field_cb = field_cb;

    results.results = malloc(results_size);
    init_test_results(results.results, result_slots);
    for (i = 0; i < num_test_cases; i++) {
        const char *str = test_data[i];
        int len = strlen(str);

        results.test_case_number = i;

        csv_parser_init(&parser);
        parser.data = &results;

        int nread = csv_parser_execute(&parser, &settings, str, len);
        ck_assert_int_eq(len, nread);
        ck_assert_int_eq(parser.state, test_data_end_states[i]);
    }

    printf("Test results:\n");
    print_results(results.results, result_slots);
    ck_assert_int_eq(0, memcmp(results.results, expected_test_results, results_size));
    free(results.results);

}
END_TEST


START_TEST(test_parser_chunked)
{
    csv_parser_t parser;
    csv_parser_settings_t settings;
    struct test_results results;
    int i, offset;

    settings.delimiter = ',';
    settings.field_cb = field_cb;

    results.results = malloc(results_size);
    init_test_results(results.results, result_slots);
    for (i = 0; i < num_test_cases; i++) {
        const char *str = test_data[i];
        int len = strlen(str);

        results.test_case_number = i;

        csv_parser_init(&parser);
        parser.data = &results;

        // parse string one char at a time
        for (offset = 0; offset < len; offset++) {
            char c = *(str+offset);
            int nread = csv_parser_execute(&parser, &settings, &c, 1);
            ck_assert_int_eq(1, nread);
        }
        ck_assert_int_eq(parser.state, test_data_end_states[i]);
    }

    printf("Test results:\n");
    print_results(results.results, result_slots);
    ck_assert_int_eq(0, memcmp(results.results, expected_test_results, results_size));
    free(results.results);

}
END_TEST


int main(int argc, const char *argv[])
{
    Suite *s = suite_create("csv_parser");
    TCase *tc = tcase_create("parser");
    tcase_add_test(tc, test_parser_full);
    tcase_add_test(tc, test_parser_chunked);

    suite_add_tcase(s, tc);
    SRunner *sr = srunner_create(s);

    printf("Expected results:\n");
    print_results(expected_test_results, result_slots);

    srunner_run_all(sr, CK_VERBOSE);
    srunner_free(sr);

    return 0;
}
