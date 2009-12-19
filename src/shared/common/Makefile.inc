COMMON_OBJS = \
    $(BUILD_DIR)/bitmasktable.o \
    $(BUILD_DIR)/sprite.o \
    $(BUILD_DIR)/tablecall.o \
    $(BUILD_DIR)/joypad.o \
    $(BUILD_DIR)/ppu.o \
    $(BUILD_DIR)/ppuwrite.o \
    $(BUILD_DIR)/ppubuffer.o \
    $(BUILD_DIR)/timer.o \
    $(BUILD_DIR)/irq.o \
    $(BUILD_DIR)/nmi.o \
    $(BUILD_DIR)/reset.o \
    $(BUILD_DIR)/vectors.o

$(BUILD_DIR)/%.o: $(BASESRCDIR)/common/%.asm
	@$(CHK_DIR_EXISTS) $(BUILD_DIR) || $(MKDIR) $(BUILD_DIR)
	$(XASM) $< -o $@
