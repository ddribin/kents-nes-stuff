;
;    Copyright (C) 2004, 2005 Kent Hansen.
;
;    This file is part of Neotoxin.
;
;    Neotoxin is free software; you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation; either version 2 of the License, or
;    (at your option) any later version.
;
;    Neotoxin is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program; if not, write to the Free Software
;    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;

; Description:
; Basic PPU stuff.
; A copy of the PPU state (control+scroll registers)
; is kept in RAM and is only transferred to PPU in the NMI (see nmi.asm).

.include "ppu.h"

.dataseg

.public ppu .ppu_state

.codeseg

; exported API
.public reset_screen
.public screen_on
.public screen_off
.public nmi_on
.public nmi_off
.public fill_nametable
.public fill_nametable_0_0
.public fill_all_nametables
.public get_scroll_xy
.public set_scroll_xy

; Selects nametable 0, sets scrollx=scrolly=0.
.proc reset_screen
    lda ppu.ctrl0
    and #$FC    ; nametable 0
    sta ppu.ctrl0
    lda #0
    sta ppu.scroll_x
    sta ppu.scroll_y
    rts
.endp

; Turns on screen (sprites and background).
; The screen won't be turned on immediately, but during next NMI
; (i.e. in order for this to work NMI has to be enabled, see
; nmi_on()).
.proc screen_on
    lda ppu.ctrl1
    ora #(PPU_CTRL1_BG_ON | PPU_CTRL1_SPRITE_ON)
    sta ppu.ctrl1
    rts
.endp

; Turns off screen.
; The screen is turned off immediately (i.e. not during NMI,
; as with screen_on()).
.proc screen_off
    lda ppu.ctrl1
    and #(~(PPU_CTRL1_BG_ON | PPU_CTRL1_SPRITE_ON) & 0xFF)
    sta ppu.ctrl1
    sta PPU_CTRL1_REG
    rts
.endp

; Turns on NMI.
.proc nmi_on
    lda ppu.ctrl0
    ora #PPU_CTRL0_NMI_ON
    sta ppu.ctrl0
    sta PPU_CTRL0_REG
    rts
.endp

; Turns off NMI.
.proc nmi_off
    lda ppu.ctrl0
    and #(~PPU_CTRL0_NMI_ON & 0xFF)
    sta ppu.ctrl0
    sta PPU_CTRL0_REG
    rts
.endp

; Fills name table with value. The corresponding attribute table
; is filled with 00s.
; Screen is assumed to be off already.
; Params:   A = tile value
;           X = name table # (0..3)
.proc fill_nametable
    pha
    lda ppu.ctrl0
    and #<~PPU_CTRL0_PPU_ADDR_INC      ; PPU increment = 1
    sta ppu.ctrl0
    sta PPU_CTRL0_REG
    txa                 ; A = nametable
    asl
    asl
    ora #$20            ; high PPU address
    ldx PPU_STATUS_REG  ; reset PPU address flip flop
    sta PPU_ADDR_REG
    lda #$00
    sta PPU_ADDR_REG
    ldy #$C0
    ldx #$04
    pla
  - sta PPU_IO_REG
    dey
    bne -
    dex
    bne -
    ldy #$40            ; attribute table size = $40 bytes
    txa                     ; A = 0
  - sta PPU_IO_REG
    dey
    bne -
    rts
.endp

.proc fill_nametable_0_0
    lda #0
    tax
    jmp fill_nametable
.endp

; Fills all name tables and their corresponding attribute tables.
; Params:   A = tile value
.proc fill_all_nametables
    pha
    ldx #0
    jsr fill_nametable
    pla
    pha
    ldx #1
    jsr fill_nametable
    pla
    pha
    ldx #2
    jsr fill_nametable
    pla
    ldx #3
    jmp fill_nametable
.endp

.proc get_scroll_xy
    lda ppu.scroll_x
    ldy ppu.scroll_y
    rts
.endp

.proc set_scroll_xy
    sta ppu.scroll_x
    sty ppu.scroll_y
    rts
.endp

.end
