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
; Basic palette stuff.
; A copy of the palette is kept in RAM so it can be faded and such
; (see fade.asm).
;  The load_palette() function loads a palette into internal memory;
; the write_palette() function writes the palette to VRAM.

.include "ptr.h"

.dataseg

; background + sprite palette (16+16 bytes)
.public palette .byte[32]

.dataseg zeropage

datap   .ptr

.codeseg

; exported API
.public load_palette
.public write_palette
.public set_palette
.public set_black_palette
.public start_palette_ppu_string

.extrn ppu_buffer:byte
.extrn ppu_buffer_offset:byte
.extrn begin_ppu_string:proc
.extrn end_ppu_string:proc

; Starts a PPU buffer string with address=$3F00 and count=32.
; Params:   A = index of first palette entry (0..31)
;           X = # of entries
; Returns:  X = PPU buffer index
.proc start_palette_ppu_string
    ldy     #$3F
    jmp     begin_ppu_string
.endp

; Loads a 32-byte palette into RAM array.
; The palette is not written to VRAM.
; Once the palette is loaded, you can call write_palette()
; or one of the palette loading functions (see fade.asm).
; Params:   A = low address of palette
;           Y = high address of palette
.proc load_palette
    sta     datap.lo
    sty     datap.hi
    ldy     #31
  - lda     [datap],y
    sta     palette,y
    dey
    bpl     -
    rts
.endp

; Writes in-RAM palette to VRAM.
; You should have set a palette first with load_palette()
; before calling this function.
; Params: None
.proc write_palette
    lda     #0
    ldx     #32
    jsr     start_palette_ppu_string
    ldy     #0
  - lda     palette,y
    iny
    sta     ppu_buffer,x
    inx
    cpy     #32
    bne     -
    jmp     end_ppu_string
.endp

; Sets all RAM palette entries to $0F (black), and writes
; the palette to VRAM.
.proc set_black_palette
    ldx     #31
    lda     #$0F    ; color black
  - sta     palette,x
    dex
    bpl     -
    bmi     write_palette
.endp

; Loads a palette and writes it to VRAM in one go.
; Params:   A = low address of palette
;           Y = high address of palette
.proc set_palette
    jsr load_palette
    jmp write_palette
.endp

.end
