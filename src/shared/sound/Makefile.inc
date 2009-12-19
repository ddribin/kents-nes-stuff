SOUND_OBJS = \
    $(BUILD_DIR)/periodtable.o \
    $(BUILD_DIR)/volumetable.o \
    $(BUILD_DIR)/envelope.o \
    $(BUILD_DIR)/effect.o \
    $(BUILD_DIR)/tonal.o \
    $(BUILD_DIR)/dmc.o \
    $(BUILD_DIR)/mixer.o \
    $(BUILD_DIR)/sequencer.o \
    $(BUILD_DIR)/sound.o

$(BUILD_DIR)/%.o: $(BASESRCDIR)/sound/%.asm
	@$(CHK_DIR_EXISTS) $(BUILD_DIR) || $(MKDIR) $(BUILD_DIR)
	$(XASM) $< -o $@
