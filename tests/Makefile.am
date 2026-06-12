AM_CPPFLAGS = -I$(top_srcdir)/src

noinst_PROGRAMS = test

test_LDFLAGS = -lcheck
test_SOURCES = main.c
test_LDADD = $(top_builddir)/src/libcsv_parser.la

TESTS = test
