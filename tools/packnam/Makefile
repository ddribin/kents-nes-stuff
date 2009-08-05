INSTALL = install
CFLAGS = -Wall -g
LFLAGS =
OBJS = packnam.o

prefix = /usr/local
datarootdir = $(prefix)/share
datadir = $(datarootdir)
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
infodir = $(datarootdir)/info
mandir = $(datarootdir)/man
docbookxsldir = /sw/share/xml/xsl/docbook-xsl

packnam: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o packnam

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: packnam
	$(INSTALL) -m 0755 packnam $(bindir)
	$(INSTALL) -m 0444 packnam.1 $(mandir)/man1

doc: packnam-refentry.docbook
	xsltproc $(docbookxsldir)/manpages/docbook.xsl $<
	xsltproc $(docbookxsldir)/html/docbook.xsl $< > doc/index.html

clean:
	rm -f $(OBJS) packnam packnam.exe

.PHONY: clean install
