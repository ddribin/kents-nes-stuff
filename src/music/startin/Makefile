PROGRAM = WannaBeStartinSomethin

SONGBASENAME = song

CHK_DIR_EXISTS = test -d
MKDIR          = mkdir -p

AFLAGS = --debug -I. -I$(BASESRCDIR) -DNO_JOYPAD1 -DNO_SFX
LFLAGS = 

XASM = xasm $(AFLAGS)
XLNK = xlnk $(LFLAGS)

BUILD_DIR = build

BASESRCDIR = ../../shared

first: all

include $(BASESRCDIR)/common/Makefile.inc
include $(BASESRCDIR)/sound/Makefile.inc

OBJS = $(COMMON_OBJS) $(SOUND_OBJS) \
    $(BUILD_DIR)/main.o \
    $(BUILD_DIR)/dmcdata.o \
    $(BUILD_DIR)/songtable.o \
    $(BUILD_DIR)/$(SONGBASENAME).o

HEADERFILE = ines.hdr
CHRFILE = graphics.chr
SCRIPTFILE = link.s
BINFILE = $(PROGRAM).nes

all: $(BUILD_DIR)/$(BINFILE)

$(BUILD_DIR)/$(BINFILE): $(OBJS) $(CHRFILE) $(SCRIPTFILE) $(HEADERFILE)
	$(XLNK) $(SCRIPTFILE) -o $@

$(BUILD_DIR)/%.o: %.asm
	@$(CHK_DIR_EXISTS) $(BUILD_DIR) || $(MKDIR) $(BUILD_DIR)
	$(XASM) $< -o $@

$(BUILD_DIR)/$(SONGBASENAME).o: $(SONGBASENAME).asm $(SONGBASENAME).inc
	@$(CHK_DIR_EXISTS) $(BUILD_DIR) || $(MKDIR) $(BUILD_DIR)
	$(XASM) $< -o $@

$(SONGBASENAME).inc: $(SONGBASENAME).xm instruments.map
	xm2nes --instruments-map=instruments.map $< > $@

.PHONY: clean

clean:
	-rm -f $(BUILD_DIR)/$(BINFILE) $(OBJS) $(SONGBASENAME).inc
