PROGRAM = Deadlock

SONGBASENAME = song

CHK_DIR_EXISTS = test -d
MKDIR          = mkdir -p

AFLAGS = --debug -I. -I$(BASESRCDIR) -DNO_SFX
LFLAGS = 

XASM = xasm $(AFLAGS)
XLNK = xlnk $(LFLAGS)

BUILD_DIR = build

BASESRCDIR = ../../shared

first: all

include $(BASESRCDIR)/sound/Makefile.inc

COMMON_OBJS = \
    $(BUILD_DIR)/bitmasktable.o \
    $(BUILD_DIR)/tablecall.o

OBJS = $(COMMON_OBJS) $(SOUND_OBJS) \
    $(BUILD_DIR)/nsfheader.o \
    $(BUILD_DIR)/dmctable.o \
    $(BUILD_DIR)/song.o \
    $(BUILD_DIR)/songtable.o

SCRIPTFILE = link.s
BINFILE = $(PROGRAM).nsf

all: $(BUILD_DIR)/$(BINFILE)

$(BUILD_DIR)/$(BINFILE): $(OBJS) $(SCRIPTFILE)
	$(XLNK) $(SCRIPTFILE) -o $@

$(BUILD_DIR)/%.o: %.asm
	@$(CHK_DIR_EXISTS) $(BUILD_DIR) || $(MKDIR) $(BUILD_DIR)
	$(XASM) $< -o $@

$(BUILD_DIR)/%.o: $(BASESRCDIR)/common/%.asm
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
