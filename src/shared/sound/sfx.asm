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
; Routines for starting and updating sound effects.

.include "mixer.h"

.dataseg zeropage

sfx .ptr

.codeseg

.public start_sfx
.public start_square_sfx
.public start_tri_sfx
.public start_noise_sfx
.public sfx_tick

.extrn mixer:mixer_state
.extrn sfx_table:ptr

; Starts playing a sound effect on given channel.
; Params:   A = SFX #
;           X = channel # * 4

.proc start_sfx
    asl
    tay
    lda sfx_table.lo,y
    sta mixer.sfx.ptr.lo,x
    lda sfx_table.hi,y
    sta mixer.sfx.ptr.hi,x
    lda #1
    sta mixer.sfx.counter,x ; triggers on next sfx_tick()
    rts
.endp

; Starts playing a sound effect on square channel 1.
; Params:   Y = *_SFX (see sfx.h)
; Destroys: A, Y

.proc start_square_sfx
    txa
    pha
    ldx #4  ; square channel 1
    go_sfx:
    tya
    jsr start_sfx
    pla
    tax
    rts
.endp

.proc start_tri_sfx
    txa
    pha
    ldx #8  ; triangle channel
    jmp go_sfx
.endp

.proc start_noise_sfx
    txa
    pha
    ldx #12 ; noise channel
    jmp go_sfx
.endp

; Does one "tick" of SFX processing.
; Params:   X = offset into sfx array

.proc sfx_tick
    dec     mixer.sfx.counter,x
    bne     ++
    lda     mixer.sfx.ptr.lo,x
    sta     sfx.lo
    lda     mixer.sfx.ptr.hi,x
    sta     sfx.hi
    ldy     #0
    lda     [sfx],y
    bne     +
; end sfx
    sta     mixer.sfx.ptr.hi,x  ; NULL
 ++ rts
  + sta     mixer.sfx.counter,x ; # of ticks until next update
    iny
    ; write new values to sound regs
    lda     [sfx],y
    sta     $4000,x ; this works since sizeof(sfx_state) is 4 bytes
    iny
    lda     [sfx],y
    sta     $4001,x
    iny
    lda     [sfx],y
    sta     $4002,x
    iny
    lda     [sfx],y
    sta     $4003,x
    iny
    ; increment SFX pointer
    tya
    clc
    adc     mixer.sfx.ptr.lo,x
    sta     mixer.sfx.ptr.lo,x
    bcc     +
    inc     mixer.sfx.ptr.hi,x
  + rts
.endp

.end
