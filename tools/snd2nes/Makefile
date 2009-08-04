INSTALL = install
CFLAGS = -Wall -g -O0
LFLAGS = -lsndfile
OBJS = main.o snd2nes.o

prefix = /usr/local
datarootdir = $(prefix)/share
datadir = $(datarootdir)
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
infodir = $(datarootdir)/info
mandir = $(datarootdir)/man
docbookxsldir = /sw/share/xml/xsl/docbook-xsl

snd2nes: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o snd2nes

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: snd2nes
	$(INSTALL) -m 0755 snd2nes $(bindir)
	$(INSTALL) -m 0444 snd2nes.1 $(mandir)/man1

doc: snd2nes-refentry.docbook
	xsltproc $(docbookxsldir)/manpages/docbook.xsl $<
	xsltproc $(docbookxsldir)/html/docbook.xsl $< > doc/index.html

clean:
	rm -f $(OBJS) snd2nes snd2nes.exe

.PHONY: clean install
