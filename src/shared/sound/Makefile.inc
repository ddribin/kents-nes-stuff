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

SOUNDSRCDIR = $(BASESRCDIR)/sound

periodtable.o: $(SOUNDSRCDIR)/periodtable.asm
	$(XASM) $< -o $@

volumetable.o: $(SOUNDSRCDIR)/volumetable.asm
	$(XASM) $< -o $@

envelope.o: $(SOUNDSRCDIR)/envelope.asm
	$(XASM) $< -o $@

effect.o: $(SOUNDSRCDIR)/effect.asm
	$(XASM) $< -o $@

tonal.o: $(SOUNDSRCDIR)/tonal.asm
	$(XASM) $< -o $@

dmc.o: $(SOUNDSRCDIR)/dmc.asm
	$(XASM) $< -o $@

sequencer.o: $(SOUNDSRCDIR)/sequencer.asm
	$(XASM) $< -o $@

mixer.o: $(SOUNDSRCDIR)/mixer.asm
	$(XASM) $< -o $@

sound.o: $(SOUNDSRCDIR)/sound.asm
	$(XASM) $< -o $@
