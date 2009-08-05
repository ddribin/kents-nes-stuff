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
; Function for writing "PPU data strings".
; It's based on a routine reverse-engineered from Metroid,
; so thank you Nintendo for that.

.include "ppu.h"
.include "ptr.h"

.dataseg zeropage

datap   .ptr

.codeseg

; exported API
.public write_ppu_data_at

.extrn ppu : ppu_state

; Writes a sequence of data strings to VRAM.
; datap: ptr to first string
; -------------
; String format:
; byte 0: high byte of PPU address
; byte 1: low byte of PPU address
; byte 2: bits 0-5 length of data string (repeat count if RLE)
;         bit 6 is data RLE? (1 = yes)
;         bit 7 PPU address increment (0 = 1, 1 = 32)
; byte 3-..: data. Only 1 byte if string is RLE
; -------------
; When byte 0 is zero, it means that no more strings remain to be written.

  - ldx     PPU_STATUS_REG  ; reset PPU address flip flop
    sta     PPU_ADDR_REG    ; set PPU hi address
    pha                     ; save it for later
    iny
    lda     [datap],y
    sta     PPU_ADDR_REG    ; set PPU lo address
    iny
    lda     [datap],y       ; string info byte
    asl                     ; CF = PPU address increment
    tax
    lda     ppu.ctrl0
    ora     #%00000100
    bcs     +               ; if CF set, PPU inc = 32
    and     #%11111011      ; else PPU inc = 1
  + sta     ppu.ctrl0
    sta     PPU_CTRL0_REG
    txa
    asl                     ; CF = RLE status (1 = RLE)
    lda     [datap],y       ; string info byte again
    and     #$3F            ; data length in lower 6 bits
    tax                     ; use as loop counter
    bcc     @@write_loop    ; branch if string isn't RLE
    iny
    lda     [datap],y       ; fetch the RLE data byte
    @@repeat_loop:
    sta     PPU_IO_REG
    dex
    bne     @@repeat_loop
    beq     +
    @@write_loop:
    iny
    lda     [datap],y       ; fetch the next data byte
    sta     PPU_IO_REG      ; write to PPU
    dex
    bne     @@write_loop
  + pla                     ; restore PPU hi address
    cmp     #$3F            ; was the data written to page $3F?
    bne     +               ; branch if not
    sta     PPU_ADDR_REG
    stx     PPU_ADDR_REG
    stx     PPU_ADDR_REG
    stx     PPU_ADDR_REG
  + tya                     ; A = string length - 1
    sec                     ; set carry, so we are adding actual strlen
    adc     datap.lo        ; add to pointer
    sta     datap.lo        ; now points to the next string
    bcc     write_ppu_string
    inc     datap.hi
    write_ppu_string:
    ldy     #0
    lda     [datap],y
    bne     -               ; non-zero = valid string, write it
    rts

; In: A = low address.
;     Y = high address.

    write_ppu_data_at:
    sta     datap.lo
    sty     datap.hi
    jmp     write_ppu_string

.end
