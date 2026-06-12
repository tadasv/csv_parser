ACLOCAL_AMFLAGS = -I config

SUBDIRS = src

DIST_SUBDIRS = src

EXTRA_DIST = \
	autogen.sh	\
	LICENSE

test: all
	cd tests ; $(MAKE) check

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = csv_parser.pc
