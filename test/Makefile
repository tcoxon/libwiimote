# test/Makefile

topdir:=..
include $(topdir)/config.mk

CFLAGS:=$(CFLAGS) -static
LIBS:=$(LIBS) -L$(LIBDIR) -lwiimote -lbluetooth -lm

all: test1 test2

test1: test1.c $(LIBDIR)/libwiimote.a
	$(GCC) $(CFLAGS) $(INCLUDES) -o $(BINDIR)/$@ $< $(LIBS)

test2: test2.c $(LIBDIR)/libwiimote.a
	$(GCC) $(CFLAGS) $(INCLUDES) -o $(BINDIR)/$@ $< $(LIBS)

clean:
	@rm -f *.o *~ $(BINDIR)/*