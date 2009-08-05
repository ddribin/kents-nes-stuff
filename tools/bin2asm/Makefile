INSTALL = install
CFLAGS = -Wall -g
LFLAGS =
OBJS = bin2asm.o

prefix = /usr/local
datarootdir = $(prefix)/share
datadir = $(datarootdir)
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
infodir = $(datarootdir)/info
mandir = $(datarootdir)/man
docbookxsldir = /sw/share/xml/xsl/docbook-xsl

bin2asm: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o bin2asm

%.o: %.c
	$(CC) -Wall -g -c $< -o $@

install: bin2asm
	$(INSTALL) -m 0755 bin2asm $(bindir)

.PHONY: clean

clean:
	rm -f $(OBJS) bin2asm bin2asm.exe
