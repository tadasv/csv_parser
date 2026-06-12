# csv_parser

This library provides a CSV parser to be used in an event loop for processing large amounts of streaming data.
The parser itself does not use any internal buffers. Whenever a field value is available, a user's specified callback will be invoked with the field data and its CSV location (row and column).

## Building & Installation

Prerequisites:
* gcc
* libtool
* autoconf
* automake
* check (to build tests)

To build and install:

```bash
$ sh autogen.sh
$ ./configure
$ make
$ make install
```

You can optionally build and run tests:

```bash
$ make test
```

## Usage

Once installed, you can link your own projects against the library using `pkg-config`:

```bash
$ gcc my_program.c -o my_program $(pkg-config --cflags --libs csv_parser)
```

### Quick Example

Here is a minimal example demonstrating how to initialize the parser and process data:

```c
#include <stdio.h>
#include <string.h>
#include <csv_parser.h>

int on_field(csv_parser_t *parser, const char *data, size_t length, int row, int col) {
    printf("Row: %d, Col: %d, Value: %.*s\n", row, col, (int)length, data);
    return 0;
}

int main() {
    csv_parser_t parser;
    csv_parser_init(&parser);

    csv_parser_settings_t settings = {
        .delimiter = ',',
        .field_cb = on_field
    };

    const char *csv_data = "name,age\nAlice,30\nBob,25";
    csv_parser_execute(&parser, &settings, csv_data, strlen(csv_data));
    csv_parser_finish(&parser, &settings);

    return 0;
}
```

See the `examples/` directory for more advanced use cases.

## Benchmarks

The library is highly optimized for throughput, avoiding internal buffering and memory allocations while parsing. We include a benchmarking tool (`examples/bench`) and several test datasets to measure parsing performance.

Recent benchmark results (run locally on an Apple Silicon machine):

| Dataset | File Size | Description | Throughput |
|---------|-----------|-------------|------------|
| `iris.csv` | 4 KB | Small, standard dataset | ~440 MB/s |
| `large.csv` | 7.6 MB | 100,000 rows, 10 columns (simple text/numbers) | ~1.3 GB/s |
| `wide.csv` | 6.6 MB | 1,000 rows, 1,000 columns | ~1.3 GB/s |
| `quoted.csv` | 23.8 MB | 100,000 rows, 10 columns with quoted strings & newlines | ~1.4 GB/s |

You can run these benchmarks yourself using:
```bash
$ ./examples/bench examples/large.csv 100
```
