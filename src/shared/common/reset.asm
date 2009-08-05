.include "ppu.h"

.ifdef MMC
.if MMC == 3
.include "../mmc/mmc3.h"
.endif
.endif

.codeseg

.extrn nmi_on:proc
.extrn init:proc
.extrn ppu:ppu_state

.public reset

reset:
    cld                     ; clear decimal mode
    sei
    ldx #$00
    stx PPU_CTRL0_REG       ; disable NMI and stuff
    stx PPU_CTRL1_REG       ; disable BG & SPR visibility and stuff
.ifdef MMC
.if MMC == 3
    stx MMC3_IRQ_DISABLE_REG
.endif
.endif
    dex                     ; X = FF
    txs                     ; S points to end of stack page (1FF)

  - lda PPU_STATUS_REG
    bpl -
  - lda PPU_STATUS_REG
    bpl -

    lda #$40
    sta $4017               ; disable erratic IRQ triggering

; clear RAM
    lda     #$00
    tax
  - sta     $00,x
    sta     $0100,x
    sta     $0200,x
    sta     $0300,x
    sta     $0400,x
    sta     $0500,x
    sta     $0600,x
    sta     $0700,x
    inx
    bne     -

; default PPU control
    lda     #(PPU_CTRL0_SPRITE_SIZE_8x8 | PPU_CTRL0_BG_TABLE_0000 | PPU_CTRL0_SPRITE_TABLE_1000)
    sta     ppu.ctrl0

; user-supplied init function
    jsr init

; on with the NMI
    jsr     nmi_on
; enable interrupts
    cli

; eternal loop, everything happens in NMI
  - jmp     -

.end
