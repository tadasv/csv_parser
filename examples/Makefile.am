AM_CPPFLAGS = -I$(top_srcdir)/src

bin_PROGRAMS = parser bench

parser_SOURCES = parser.c
parser_LDADD = $(top_builddir)/src/libcsv_parser.la

bench_SOURCES = bench.c
bench_LDADD = $(top_builddir)/src/libcsv_parser.la
