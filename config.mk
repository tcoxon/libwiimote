# config.mk

VERSION=0.3.0

BINDIR=$(topdir)/bin
LIBDIR=$(topdir)/lib
SRCDIR=$(topdir)/src

INSTALL_PREFIX=/usr/local
INSTALL_BINDIR=$(INSTALL_PREFIX)/bin
INSTALL_LIBDIR=$(INSTALL_PREFIX)/lib
INSTALL_INCDIR=$(INSTALL_PREFIX)/include/libcwiimote-$(VERSION)

GCC:=gcc
#CFLAGS:=-O3 -march=athlon-xp -Wall -pipe
CFLAGS:=-Os -Wall -pipe -D_ENABLE_TILT -D_ENABLE_FORCE -DVERSION=$(VERSION)
INCLUDES:=-I$(topdir)/src
LIBS:=

VPATH=:$(BINDIR) $(LIBDIR)
