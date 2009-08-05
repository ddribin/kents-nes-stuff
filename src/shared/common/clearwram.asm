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

.dataseg

ptr .dw

.codeseg

.public clear_wram

; Clears WRAM at $6000-$7FFF
; Params: None
; Destroys: A, X

.proc clear_wram
    lda     #$7F        ; high byte of last WRAM page
    sta     ptr+1
    lda     #$00        ; low byte of last WRAM page
    sta     ptr
    tay
  - sta     [ptr],y     ; clear address
    iny                 ; cleared 256 bytes?
    bne     -           ; if not, clear another
    dec     ptr+1       ; decrement high byte of address
    ldx     ptr+1
;        cpx     #$60   ; HMM???
    cpx     #$70
    bcs     -           ; clear another page if >= $60
    rts
.endp

.end
