.include "ppu.h"
.include "sprite.h"
.include "tablecall.h"

.ifdef MMC
.if MMC == 3
.include "../mmc/mmc3.h"
.endif
.endif

.dataseg

.extrn ppu:ppu_state
.extrn sprites:byte

.ifdef MMC
.if MMC == 3
.extrn chr_banks:byte
.extrn irq_count:byte
.endif
.endif

in_nmi .byte
frame_count .byte
main_cycle .byte

.public frame_count
.public main_cycle

.codeseg

.public nmi

.extrn flush_ppu_buffer:proc
.extrn read_joypad:proc
.ifndef NO_JOYPAD1
.extrn read_joypad1:proc
.endif
.ifndef NO_SOUND
.extrn update_sound:proc
.endif
.extrn table_call:proc
.extrn update_timers:proc

nmi:
    sei
    pha                     ; preserve A
    txa
    pha                     ; preserve X
    tya
    pha                     ; preserve Y

    lda     ppu.ctrl1
    sta     PPU_CTRL1_REG

    lda     in_nmi
    bne     skip_nmi        ; skip the next part if the frame couldn't
                            ; finish before the NMI was triggered
    inc     in_nmi
    inc     frame_count

    jsr     flush_ppu_buffer

; update PPU control register 0
    lda     ppu.ctrl0
    sta     PPU_CTRL0_REG

; update scroll registers
    lda     PPU_STATUS_REG  ; reset H/V scroll flip flop
    lda     ppu.scroll_x
    sta     PPU_SCROLL_REG
    lda     ppu.scroll_y
    sta     PPU_SCROLL_REG

; perform sprite DMA
    lda     #0
    sta     SPRITE_ADDR_REG ; reset SPR-RAM address
    lda     #>sprites
    sta     SPRITE_DMA_REG

.ifdef MMC
.if MMC == 3
; set CHR banks
    ldx     #$05
  - stx     MMC3_CTRL_REG
    lda     chr_banks,x
    sta     MMC3_PAGE_REG
    dex
    bpl     -

; set up IRQ if requested
    lda     irq_count
    beq     +
    sta     MMC3_IRQ_COUNTER_REG
    sta     MMC3_IRQ_LATCH_REG
    sta     MMC3_IRQ_ENABLE_REG
  +
.endif
.endif

; read joypad(s)
    jsr     read_joypad
.ifndef NO_JOYPAD1
    jsr     read_joypad1
.endif

.ifndef NO_SOUND
    jsr     update_sound
.endif

; update timers
    jsr     update_timers

    jsr     go_main_cycle

    dec     in_nmi          ; = 0, NMI done
    skip_nmi:
    pla
    tay                     ; restore Y
    pla
    tax                     ; restore X
    pla                     ; restore A
    rti

go_main_cycle:
    lda     main_cycle
    jsr     table_call
.include "maincycletable.asm"

.end
