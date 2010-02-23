SUBDIRS = src scripts data po themes help

install-exec-hook:
	mkdir -p ${DESTDIR}/usr/share/help/eabout
	oi-help-compiler -u $(srcdir)/help ${DESTDIR}/usr/share/help/eabout
