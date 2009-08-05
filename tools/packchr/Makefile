INSTALL = install
CFLAGS = -Wall -g
LFLAGS =
OBJS = packchr.o

prefix = /usr/local
datarootdir = $(prefix)/share
datadir = $(datarootdir)
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
infodir = $(datarootdir)/info
mandir = $(datarootdir)/man
docbookxsldir = /sw/share/xml/xsl/docbook-xsl

packchr: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o packchr

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: packchr
	$(INSTALL) -m 0755 packchr $(bindir)
	$(INSTALL) -m 0444 packchr.1 $(mandir)/man1

doc: packchr-refentry.docbook
	xsltproc $(docbookxsldir)/manpages/docbook.xsl $<
	xsltproc $(docbookxsldir)/html/docbook.xsl $< > doc/index.html

clean:
	rm -f $(OBJS) packchr packchr.exe

.PHONY: clean install
