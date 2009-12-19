COMMON_OBJS = \
    bitmasktable.o \
    sprite.o \
    tablecall.o \
    joypad.o \
    ppu.o \
    ppuwrite.o \
    ppubuffer.o \
    timer.o \
    irq.o \
    nmi.o \
    reset.o \
    vectors.o

COMMONSRCDIR = $(BASESRCDIR)/common

timer.o: $(COMMONSRCDIR)/timer.asm
	$(XASM) $< -o $@

bitmasktable.o: $(COMMONSRCDIR)/bitmasktable.asm
	$(XASM) $< -o $@

sprite.o: $(COMMONSRCDIR)/sprite.asm
	$(XASM) $< -o $@

tablecall.o: $(COMMONSRCDIR)/tablecall.asm
	$(XASM) $< -o $@

joypad.o: $(COMMONSRCDIR)/joypad.asm
	$(XASM) $< -o $@

ppu.o: $(COMMONSRCDIR)/ppu.asm
	$(XASM) $< -o $@

ppuwrite.o: $(COMMONSRCDIR)/ppuwrite.asm
	$(XASM) $< -o $@

ppubuffer.o: $(COMMONSRCDIR)/ppubuffer.asm
	$(XASM) $< -o $@

irq.o: $(COMMONSRCDIR)/irq.asm
	$(XASM) $< -o $@

nmi.o: $(COMMONSRCDIR)/nmi.asm maincycletable.asm
	$(XASM) $< -o $@

reset.o: $(COMMONSRCDIR)/reset.asm
	$(XASM) $< -o $@

vectors.o: $(COMMONSRCDIR)/vectors.asm
	$(XASM) $< -o $@
