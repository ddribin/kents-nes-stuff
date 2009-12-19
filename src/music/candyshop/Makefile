PROGRAM = candy

CHK_DIR_EXISTS = test -d
MKDIR          = mkdir -p

# assembler (xasm) flags
AFLAGS = --debug -I. -I$(BASESRCDIR) -DNO_JOYPAD1 -DNO_SFX

# linker (xlnk) flags
LFLAGS = 

# command to invoke assembler
XASM = xasm $(AFLAGS)

# command to invoke linker
XLNK = xlnk $(LFLAGS)

BUILD_DIR = build

# the path to the directory that contains the shared source code
BASESRCDIR = ../../shared

first: all

include $(BASESRCDIR)/common/Makefile.inc
include $(BASESRCDIR)/sound/Makefile.inc

OBJS = $(COMMON_OBJS) $(SOUND_OBJS) \
    $(BUILD_DIR)/main.o \
    $(BUILD_DIR)/dmcdata.o \
    $(BUILD_DIR)/songtable.o \
    $(BUILD_DIR)/$(PROGRAM).o

# the name of the 16-byte iNES header binary file
HEADERFILE = $(PROGRAM).hdr

CHRFILE = $(PROGRAM).chr

# the name of the linker script
SCRIPTFILE = $(PROGRAM).s

# the name of the final binary (NES ROM)
BINFILE = $(PROGRAM).nes

all: $(BUILD_DIR)/$(BINFILE)

$(BUILD_DIR)/$(BINFILE): $(OBJS) $(CHRFILE) $(SCRIPTFILE)
	$(XLNK) $(SCRIPTFILE)

$(BUILD_DIR)/%.o: %.asm
	@$(CHK_DIR_EXISTS) $(BUILD_DIR) || $(MKDIR) $(BUILD_DIR)
	$(XASM) $< -o $@

$(BUILD_DIR)/$(PROGRAM).o: $(PROGRAM).asm $(PROGRAM).inc
	@$(CHK_DIR_EXISTS) $(BUILD_DIR) || $(MKDIR) $(BUILD_DIR)
	$(XASM) $< -o $@

$(PROGRAM).inc: $(PROGRAM).xm instruments.map
	xm2nes --instruments-map=instruments.map $< > $@

.PHONY: clean

clean:
	-rm -f $(BUILD_DIR)/$(BINFILE) $(OBJS) $(PROGRAM).inc
