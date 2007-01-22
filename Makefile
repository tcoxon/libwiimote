# Makefile

topdir=.
include $(topdir)/config.mk

subdirs=src test

all: subdirs

subdirs:
	@for dir in $(subdirs); do \
		(cd $$dir && make) || exit 1; \
	done

clean:
	@rm -f bin/* lib/*
	@for dir in $(subdirs); do \
		(cd $$dir && make clean) || exit 1; \
	done
