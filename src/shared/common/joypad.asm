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
; Reads joypad.

.include "joypad.h"

.dataseg

.public joypad0         .joypad
.public joypad0_posedge .joypad
.ifndef NO_JOYPAD1
.public joypad1         .joypad
.public joypad1_posedge .joypad
.endif

.define READ_TWICE

.codeseg

.public read_joypad
.ifndef NO_JOYPAD1
.public read_joypad1
.endif

; Reads the status of joypad 0.
; Stores the result in the global variable joypad0.

.proc read_joypad
    lda     joypad0
    pha
.ifdef READ_TWICE
    lda     #0
    sta     joypad0
    @@again:
    lda     joypad0
    pha
.endif
    ldy     #1
    sty     JOYPAD0_IO_REG  ; reset strobe
    dey
    sty     JOYPAD0_IO_REG  ; clear strobe
    ldy     #8              ; do all 8 buttons       
  - lda     JOYPAD0_IO_REG  ; load button status
    lsr                     ; transfer to carry flag
    rol     joypad0         ; rotate all bits left, put CF in bit 0
    dey                     ; done 8 buttons?
    bne     -               ; if not, do another
    pla
.ifdef READ_TWICE
    cmp     joypad0
    bne     @@again
    pla
.endif
    eor     joypad0
    and     joypad0
    sta     joypad0_posedge
    rts
.endp

.ifndef NO_JOYPAD1

; Reads the status of joypad 1.
; Stores the result in the global variable joypad1.

.proc read_joypad1
    lda     joypad1
    pha
.ifdef READ_TWICE
    lda     #0
    sta     joypad1
    @@again:
    lda     joypad1
    pha
.endif
    ldy     #1
    sty     JOYPAD0_IO_REG  ; reset strobe
    dey
    sty     JOYPAD0_IO_REG  ; clear strobe
    ldy     #8              ; do all 8 buttons       
  - lda     JOYPAD1_IO_REG  ; load button status
    lsr                     ; transfer to carry flag
    rol     joypad1         ; rotate all bits left, put CF in bit 0
    dey                     ; done 8 buttons?
    bne     -               ; if not, do another
    pla
.ifdef READ_TWICE
    cmp     joypad1
    bne     @@again
    pla
.endif
    eor     joypad1
    and     joypad1
    sta     joypad1_posedge
    rts
.endp

.endif

.end
