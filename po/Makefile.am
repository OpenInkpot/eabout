PACKAGE = @PACKAGE@

POFILES = $(wildcard $(srcdir)/*.po)
MOFILES = $(patsubst %.po,%.mo,$(notdir $(POFILES)))

top_builddir = ..
localedir=$(datadir)/locale

MSGFMT = msgfmt

update-pot:
	rm -f $(PACKAGE).pot
	xgettext -k_ -d $(PACKAGE) \
		--foreign-user \
		--add-comments \
		--package-name="$(PACKAGE)" --package-version="@VERSION@" \
		--msgid-bugs-address="@PACKAGE_BUGREPORT@" \
		--from-code=UTF-8 \
		-f POTFILES -D $(top_srcdir) -o $(PACKAGE).pot

update-po:
	for cat in "" $(POFILES); do \
	  if test -z "$$cat"; then continue; fi; \
	  msgmerge -o tmp-$$cat $$cat $(PACKAGE).pot && \
		mv -f tmp-$$cat $$cat; \
	done
	

all: $(MOFILES)

%.mo: %.po
	@echo "$(MSGFMT) -o $@ $<"; $(MSGFMT) -o t-$@ $< && mv t-$@ $@

install-data-hook: $(MOFILES)
	@for cat in "" $(MOFILES); do \
	  if test -z "$$cat"; then continue; fi; \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed -e 's/\.mo$$//'`; \
	  dir=$(localedir)/$$lang/LC_MESSAGES; \
	  $(mkdir_p) $(DESTDIR)$$dir; \
	  if test -r $$cat; then realcat=$$cat; else realcat=$(srcdir)/$$cat; fi; \
	  $(INSTALL) $$realcat $(DESTDIR)$$dir/$(PACKAGE).mo; \
	 done
