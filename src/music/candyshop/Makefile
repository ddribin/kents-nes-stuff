# the path to the directory that contains the shared source code
BASESRCDIR = ../../shared

# assembler (xasm) flags
AFLAGS = --debug -I. -I$(BASESRCDIR) -DNO_SFX

# linker (xlnk) flags
LFLAGS = 

# command to invoke assembler
XASM = xasm $(AFLAGS)

# command to invoke linker
XLNK = xlnk $(LFLAGS)

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

SOUND_OBJS = \
    periodtable.o \
    volumetable.o \
    envelope.o \
    effect.o \
    tonal.o \
    dmc.o \
    mixer.o \
    sequencer.o \
    sound.o

OBJS = $(COMMON_OBJS) $(SOUND_OBJS) \
    main.o \
    dmcdata.o \
    songtable.o \
    candy.o

PROGRAM = candy

# the name of the 16-byte iNES header binary file
HEADERFILE = $(PROGRAM).hdr

# the name of the linker script
SCRIPTFILE = $(PROGRAM).s

# the name of the final binary (NES ROM)
BINFILE = $(PROGRAM).nes

$(BINFILE): $(OBJS) $(SCRIPTFILE)
	$(XLNK) $(SCRIPTFILE)

periodtable.o: $(BASESRCDIR)/sound/periodtable.asm
	$(XASM) $< -o $@

volumetable.o: $(BASESRCDIR)/sound/volumetable.asm
	$(XASM) $< -o $@

envelope.o: $(BASESRCDIR)/sound/envelope.asm
	$(XASM) $< -o $@

effect.o: $(BASESRCDIR)/sound/effect.asm
	$(XASM) $< -o $@

tonal.o: $(BASESRCDIR)/sound/tonal.asm
	$(XASM) $< -o $@

dmc.o: $(BASESRCDIR)/sound/dmc.asm
	$(XASM) $< -o $@

sequencer.o: $(BASESRCDIR)/sound/sequencer.asm
	$(XASM) $< -o $@

mixer.o: $(BASESRCDIR)/sound/mixer.asm
	$(XASM) $< -o $@

sound.o: $(BASESRCDIR)/sound/sound.asm
	$(XASM) $< -o $@

timer.o: $(BASESRCDIR)/common/timer.asm
	$(XASM) $< -o $@

bitmasktable.o: $(BASESRCDIR)/common/bitmasktable.asm
	$(XASM) $< -o $@

sprite.o: $(BASESRCDIR)/common/sprite.asm
	$(XASM) $< -o $@

tablecall.o: $(BASESRCDIR)/common/tablecall.asm
	$(XASM) $< -o $@

joypad.o: $(BASESRCDIR)/common/joypad.asm
	$(XASM) $< -o $@

ppu.o: $(BASESRCDIR)/common/ppu.asm
	$(XASM) $< -o $@

ppuwrite.o: $(BASESRCDIR)/common/ppuwrite.asm
	$(XASM) $< -o $@

ppubuffer.o: $(BASESRCDIR)/common/ppubuffer.asm
	$(XASM) $< -o $@

irq.o: $(BASESRCDIR)/common/irq.asm
	$(XASM) $< -o $@

nmi.o: $(BASESRCDIR)/common/nmi.asm maincycletable.asm
	$(XASM) $< -o $@

reset.o: $(BASESRCDIR)/common/reset.asm
	$(XASM) $< -o $@

vectors.o: $(BASESRCDIR)/common/vectors.asm
	$(XASM) $< -o $@

main.o: main.asm candy.dat
	$(XASM) $< -o $@

dmcdata.o: dmcdata.asm
	$(XASM) $< -o $@

songtable.o: songtable.asm
	$(XASM) $< -o $@

candy.o: candy.asm candy.inc
	$(XASM) $< -o $@

candy.inc: CANDY.XM instruments.map
	xm2nes --instruments-map=instruments.map $< > $@

.PHONY: clean

clean:
	-rm -f $(BINFILE) $(OBJS) candy.inc
