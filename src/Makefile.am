bin_PROGRAMS = eabout
eabout_SOURCES = main.c script.c packages.c

AM_CFLAGS = --std=gnu99 -W -Wall \
    -DTHEME_DIR='"$(pkgdatadir)/themes/"' \
	-DPKGLIBDIR='"$(pkglibdir)"'

INCLUDES = @EFL_CFLAGS@ @LIBKEYS_CFLAGS@ @LIBEOI_CFLAGS@ @LOPS_CFLAGS@
eabout_LDFLAGS = -Wl,--as-needed
eabout_LDADD = @EFL_LIBS@ @LIBKEYS_LIBS@ @LIBEOI_LIBS@ @LOPS_LIBS@
