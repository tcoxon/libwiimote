# Makefile

topdir=.
include $(topdir)/config.mk

subdirs=src test

all: subdirs

subdirs:
	@for dir in $(subdirs); do \
		(cd $$dir && make) || exit 1; \
	done

install:
	install -D $(LIBDIR)/libcwiimote.so  $(INSTALL_LIBDIR)/libcwiimote.so.$(VERSION) -m 755
	install -D $(LIBDIR)/libcwiimote.a $(INSTALL_LIBDIR)/libcwiimote.a -m 755
	install -d $(INSTALL_INCDIR)/libcwiimote -m 755
	install -t $(INSTALL_INCDIR)/libcwiimote $(SRCDIR)/*.h -m 644

uninstall:
	rm -rf $(INSTALL_INCDIR)
	rm -f  $(INSTALL_LIBDIR)/libcwiimote.a
	rm -f  $(INSTALL_LIBDIR)/libcwiimote.so.$(VERSION)

clean:
	@rm -f *~
	@rm -f bin/* lib/*
	@for dir in $(subdirs); do \
		(cd $$dir && make clean) || exit 1; \
	done
