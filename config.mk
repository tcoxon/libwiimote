# config.mk

VERSION=0.3.0

BINDIR=$(topdir)/bin
LIBDIR=$(topdir)/lib
SRCDIR=$(topdir)/src

GCC:=gcc
DEFS:=-D_ENABLE_TILT -D_ENABLE_FORCE #-D_DISABLE_NONBLOCK_UPDATES -D_DISABLE_AUTO_SELECT_DEVICE
#CFLAGS:=-O3 -march=athlon-xp -Wall -pipe
CFLAGS:=-Os -Wall -pipe $(DEFS) -DVERSION=$(VERSION)
INCLUDES:=-I$(topdir)/src
LIBS:=

VPATH=:$(BINDIR) $(LIBDIR)
