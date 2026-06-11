#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <csv_parser.h>

int noop_field_cb(csv_parser_t *parser, const char *data, size_t length, int row, int col)
{
    return 0; // Do nothing, we only want to benchmark the parser, not allocation overhead
}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <csv_file> [iterations]\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int iterations = 1000;
    if (argc >= 3) {
        iterations = atoi(argv[2]);
    }

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t file_size = st.st_size;
    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        perror("malloc");
        close(fd);
        return 1;
    }

    ssize_t nread = read(fd, buffer, file_size);
    if (nread != file_size) {
        perror("read");
        free(buffer);
        close(fd);
        return 1;
    }
    close(fd);

    csv_parser_t parser;
    csv_parser_settings_t settings;
    settings.delimiter = ',';
    settings.field_cb = noop_field_cb;

    printf("Benchmarking %s (%.2f MB) for %d iterations...\n", filename, (double)file_size / (1024 * 1024), iterations);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iterations; i++) {
        csv_parser_init(&parser);
        csv_parser_execute(&parser, &settings, buffer, file_size);
        csv_parser_finish(&parser, &settings);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    free(buffer);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double total_bytes = (double)file_size * iterations;
    double mb_per_sec = (total_bytes / (1024 * 1024)) / elapsed;

    printf("Time elapsed: %.3f seconds\n", elapsed);
    printf("Throughput: %.2f MB/s\n", mb_per_sec);

    return 0;
}
