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
; Real-time volume envelope processing.
; Only useful for channels 0, 1 and 3.
; Volume envelope format:
; Byte 0: Start volume (0..15)
; Series of 4-byte tuples: 8x8 fixed-point step, hold length, end volume
; Until integer part of step = $FF
; If next byte is also $FF, it's really the end
; Otherwise the envelope is looped from that value

.include "mixer.h"

.dataseg zeropage

; private variables
env .ptr

.codeseg

; exported API
.public envelope_tick

; external symbols
.extrn mixer:mixer_state

; Copies current volume envelope data pointer to local pointer.
; Params:  X = channel number
.macro set_env_ptr
    lda     mixer.envelopes.ptr.lo,x
    sta     env.lo
    lda     mixer.envelopes.ptr.hi,x
    sta     env.hi         ; set pointer to envelope
.endm

; Fetches next byte from volume envelope data.
; Params:  Y = envelope data offset (auto-incremented)
.macro fetch_env_byte
    lda     [env],y
    iny
.endm

; Does one tick of envelope processing.
; Params:  X = channel number

.proc envelope_tick
    lda     mixer.envelopes.phase,x
    bmi     @@init                ; phase = $80
    asl
    bmi     @@process             ; phase = $40
    asl
    bmi     @@sustain             ; phase = $20
    rts

    ; Initialize envelope
    @@init:
    lsr     mixer.envelopes.phase,x         ; phase = update envelope
    lda     #0
    sta     mixer.envelopes.pos,x           ; reset envelope position
    tay
    set_env_ptr
    @@init_vol:
    fetch_env_byte
    sta     mixer.envelopes.vol.int,x        ; 1st byte = start volume
    ; Initialize envelope point
    @@point_init:
    fetch_env_byte
    cmp     #$FF                    ; $FF = end of envelope reached
    beq     @@env_end
    ; Point OK, set 4-tuple
    sta     mixer.envelopes.step.int,x
    fetch_env_byte
    sta     mixer.envelopes.step.frac,x
    fetch_env_byte
    sta     mixer.envelopes.hold,x
    fetch_env_byte
    sta     mixer.envelopes.dest,x
    lda     #$00
    sta     mixer.envelopes.vol.frac,x
    tya
    sta     mixer.envelopes.pos,x
    rts
    ; End of envelope reached (step.int = FF)
    @@env_end:
    fetch_env_byte  ; if FF, definitely end... otherwise loop
    cmp     #$FF
    beq     @@env_stop
    tay     ; loop the envelope
    jmp     @@init_vol
    ; No more envelope processing
    @@env_stop:
    lda     #$00
    sta     mixer.envelopes.phase,x
  - rts

    ; Sustain volume until hold == 0.
    @@sustain:
    ldy     mixer.envelopes.hold,x
    iny
    beq     -                       ; sustain forever if length = $FF
    dec     mixer.envelopes.hold,x
    bne     -
    asl     mixer.envelopes.phase,x             ; back to phase = process
    jmp     @@next_point

    ; Update volume according to step
    @@process:
    ; fractional part
    lda     mixer.envelopes.vol.frac,x
    clc
    adc     mixer.envelopes.step.frac,x
    sta     mixer.envelopes.vol.frac,x
    php
    ; integer part
    lda     mixer.envelopes.vol.int,x
    cmp     mixer.envelopes.dest,x
    bcs     @@sub_volume
; CurrentVol < DestVol ==> CurrentVol += StepHi
    plp
    adc     mixer.envelopes.step.int,x
    cmp     mixer.envelopes.dest,x
    bcs     @@reached_dest
    sta     mixer.envelopes.vol.int,x
    rts
; CurrentVol > DestVol ==> CurrentVol -= StepHi
    @@sub_volume:
    plp
    rol
    eor     #$01            ; toggle carry
    ror
    sbc     mixer.envelopes.step.int,x
    bmi     @@reached_dest
    cmp     mixer.envelopes.dest,x
    beq     @@reached_dest
    bcc     @@reached_dest
    sta     mixer.envelopes.vol.int,x
    rts
    ; Reached point's destination volume
    @@reached_dest:
    lda     mixer.envelopes.dest,x
    sta     mixer.envelopes.vol.int,x
    lda     mixer.envelopes.hold,x
    beq     @@next_point
    lsr     mixer.envelopes.phase,x             ; phase = sustain
    rts
    ; Start the next envelope point
    @@next_point:
    set_env_ptr
    ldy     mixer.envelopes.pos,x
    jmp     @@point_init
.endp

.end
