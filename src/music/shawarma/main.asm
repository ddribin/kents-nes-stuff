;
;    Copyright (C) 2009 Kent Hansen.
;
;    This program is free software; you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation; either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

.include "common/sprite.h"
.include "common/ppu.h"
.include "sound/mixer.h"

.dataseg

.extrn ppu:ppu_state
.extrn sprites:sprite_state
.extrn mixer:mixer_state
.extrn sound_status:byte

temp .byte
save_x .byte
save_y .byte

lo .byte
hi .byte
mid .byte
save_period_lo .byte
save_period_hi .byte
save_period_index .byte

offset .byte
temp_sound_status .byte
y_pos .byte

.codeseg

.extrn period_table_lo:byte
.extrn period_table_hi:byte
.extrn volume_table:byte
.extrn reset_sprites:proc
.extrn next_sprite_index:proc

.public init
.public test_cycle

.extrn fill_all_nametables:proc
.extrn write_ppu_data_at:proc
.extrn screen_on:proc
.extrn start_song:proc
.extrn mixer_get_muted_channels:proc
.extrn mixer_set_muted_channels:proc
.extrn joypad0_posedge:byte

.proc init
    lda ppu.ctrl0
    ora #PPU_CTRL0_SPRITE_SIZE_8x16
    sta ppu.ctrl0
    lda #0
    jsr fill_all_nametables
    lda #<@@ppu_data
    ldy #>@@ppu_data
    jsr write_ppu_data_at
    lda #1
    jsr start_song
    jsr screen_on
    rts
@@ppu_data:
.charmap "font.tbl"
;.incbin "packnam.dat"
.db $22,$69,13
.char "LEGEND OF THE"
.db $22,$89,14
.char "BLACK SHAWARMA"
.db $22,$C8,17
.char "ORIGINAL ALBUM BY"
.db $22,$E8,17
.char "INFECTED MUSHROOM"
.db $23,$25,23
.char "MEDLEY BY SNOWBRO, 2009"
.db $23,$49,14
.char "MADE IN NORWAY"
.db $3F,$00,$20
.db $0F,$16,$27,$20
.db $0F,$00,$10,$20
.db $0F,$00,$10,$20
.db $0F,$00,$10,$20
.db $0F,$02,$12,$22
.db $0F,$06,$16,$26
.db $0F,$09,$19,$29
.db $0F,$0C,$1C,$2C
.db 0
.endp

;.define USE_BINARY_SEARCH
.proc find_closest_period_index
    sta save_period_lo
    sty save_period_hi
.ifndef USE_BINARY_SEARCH
    ldy #0
  - lda save_period_lo
    sec
    sbc period_table_lo,y
    lda save_period_hi
    sbc period_table_hi,y
    beq +
    bcs +
    iny
    bne -
  + tya
.else ; binary search
    lda #0
    sta lo
    lda #74 ; table has 75 entries
    sta hi
    @@loop:
    lda hi
    cmp lo
    bcc @@done
    clc
    adc lo
    lsr
    sta mid
    tay
    lda period_table_lo,y
    sec
    sbc save_period_lo
    lda period_table_hi,y
    sbc save_period_hi
    beq @@done
    bcc @@less
    lda mid
    sta lo
    inc lo
    jmp @@loop
    @@less:
    lda mid
    sta hi
    dec hi
    jmp @@loop
    @@done:
    lda mid
.endif
    rts
.endp

; A = shape in lower 3 bits, palette in upper 2 bits
; X = x coordinate
; Y = y coordinate
.proc draw_thing
    stx save_x
    sty save_y
    ; first half
    pha
    jsr next_sprite_index
    tay
    pla
    pha
    and #$07
    asl
    asl
    ora #$01
    sta sprites.tile,y
    lda save_x
    sta sprites._x,y
    lda save_y
    sta sprites._y,y
    pla
    pha
    rol
    rol
    rol
    and #$03
    sta sprites.attr,y
    ; second half
    jsr next_sprite_index
    tay
    pla
    pha
    asl
    asl
    ora #$03
    sta sprites.tile,y
    lda save_x
    clc
    adc #8
    sta sprites._x,y
    lda save_y
    sta sprites._y,y
    pla
    rol
    rol
    rol
    and #$03
    sta sprites.attr,y
    rts
.endp

.proc visualize_sound
    lda sound_status
    sta temp_sound_status
    lda #24
    sta y_pos
    lda #0
    sta offset
    @@visualize_loop:
    ldx offset
.ifndef NO_MUTABLE_CHANNELS
    lsr temp_sound_status
.endif
    lda mixer.envelopes.master,x
.ifndef NO_MUTABLE_CHANNELS
    bcc +
    lda #0 ; the channel is muted
  +
.endif
    and #$F0
    ora mixer.envelopes.vol.int,x
    tay
    lda volume_table,y
    ora mixer.master_vol
    tay
    lda volume_table,y

    cmp #8
    bcc +
    lda #7
  + sta temp
    lda mixer.tonals.instrument,x ; lower 2 bits used as palette index
    ror
    ror
    ror
    and #$C0
    ora temp
    sta temp

    lda mixer.tonals.period.lo,x
    ora mixer.tonals.period.hi,x
    beq @@skip
 
    lda mixer.tonals.period.lo,x
    ldy mixer.tonals.period.hi,x
    jsr find_closest_period_index
    sta save_period_index
;    lda mixer.tonals.period_index,x
    asl
    adc save_period_index
;    adc mixer.tonals.period_index,x
    adc #48
    tax
    lda temp
    ldy y_pos
    jsr draw_thing

    @@skip:
    lda y_pos
    clc
    adc #32
    sta y_pos

    lda offset
    clc
    adc #sizeof tonal_state
    sta offset
    cmp #(4 * sizeof tonal_state)
    bne @@visualize_loop
    rts
.endp

.proc test_cycle
    jsr reset_sprites
    jsr visualize_sound
    lda joypad0_posedge
    beq +
    jsr mixer_get_muted_channels
    eor joypad0_posedge
    and #$0F
    jsr mixer_set_muted_channels
  + rts
.endp

.end
