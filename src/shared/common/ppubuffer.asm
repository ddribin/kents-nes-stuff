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
; An area of RAM is used to buffer data that should be written to PPU.
; The buffer is typically flushed during NMI, when it is safe to write
; (see ppu.asm).
; The buffer can be populated by strings of bytes of the format described
; in ppuwrite.asm.

.dataseg

.ifndef PPU_BUFFER_SIZE
.define PPU_BUFFER_SIZE 128
.endif

ppu_buffer  .byte[PPU_BUFFER_SIZE]

.dataseg zeropage

ppu_buffer_offset   .byte

.public ppu_buffer_offset
.public ppu_buffer

.codeseg

; exported API
.public reset_ppu_buffer
.public flush_ppu_buffer
.public end_ppu_string
.public begin_ppu_string
.public put_ppu_string_byte

.extrn write_ppu_data_at:proc

; Writes any data waiting to be written to VRAM.

.proc flush_ppu_buffer
    lda     ppu_buffer_offset
    beq     +               ; exit if no data pending
    lda     #0
    sta     ppu_buffer_offset

    lda     #<ppu_buffer
    ldy     #>ppu_buffer
    jmp     write_ppu_data_at
.endp

; Ends a PPU string.
; Assumes that X points _past_ the last byte of data to be written.

.proc end_ppu_string
    stx     ppu_buffer_offset
    lda     #0
    sta     ppu_buffer,x
  + rts
.endp

; Resets the PPU buffer state.

.proc reset_ppu_buffer
    lda     #0
    sta     ppu_buffer_offset
    rts
.endp

; Starts a PPU string.
; Params:  A, Y = address
;          X = count
; Returns: X = next index into PPU buffer
.proc begin_ppu_string
    pha
    txa
    pha
    ldx     ppu_buffer_offset
    tya
    sta     ppu_buffer+0,x
    pla
    sta     ppu_buffer+2,x
    pla
    sta     ppu_buffer+1,x
    inx
    inx
    inx
    rts
.endp

; Writes a byte to the PPU buffer and increments the index.
; Params:  A = byte to write
;          X = index
; Returns: X = index + 1
.proc put_ppu_string_byte
    sta     ppu_buffer,x
    inx
    rts
.endp

.end
