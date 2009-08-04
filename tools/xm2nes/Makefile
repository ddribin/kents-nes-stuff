INSTALL = install
CFLAGS = -Wall -g -O0
LFLAGS =
OBJS = xm2nes.o xm.o main.o

prefix = /usr/local
datarootdir = $(prefix)/share
datadir = $(datarootdir)
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
infodir = $(datarootdir)/info
mandir = $(datarootdir)/man
docbookxsldir = /sw/share/xml/xsl/docbook-xsl

xm2nes: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o xm2nes

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: xm2nes
	$(INSTALL) -m 0755 xm2nes $(bindir)
	$(INSTALL) -m 0444 xm2nes.1 $(mandir)/man1

doc: xm2nes-refentry.docbook
	xsltproc $(docbookxsldir)/manpages/docbook.xsl $<
	xsltproc $(docbookxsldir)/html/docbook.xsl $< > doc/index.html

clean:
	rm -f $(OBJS) xm2nes xm2nes.exe

.PHONY: clean install
