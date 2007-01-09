# config.mk

BINDIR=$(topdir)/bin
LIBDIR=$(topdir)/lib

GCC:=gcc
#CFLAGS:=-O3 -march=athlon-xp -Wall -pipe
CFLAGS:=-Os -Wall -pipe
INCLUDES:=-I$(topdir)/src
LIBS:=

VPATH=:$(BINDIR) $(LIBDIR)