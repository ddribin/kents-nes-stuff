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
; MOD-style music effect generators.
; Each effect modifies the period in a certain way.
; - Slide up/down
; - Portamento
; - Vibrato
; - Arpeggio

.include "mixer.h"
.include "track.h"
.include <common/tablecall.h>

.dataseg

temp    .int16
value   .byte

.codeseg

; exported API
.public effect_tick

; external symbols
.extrn period_table_lo:byte
.extrn period_table_hi:byte
.extrn mixer:mixer_state
.extrn tracks:track_state
.extrn table_call:proc

; Do one tick of effect.
; Params:  X = channel number

.proc effect_tick
    lda mixer.tonals.effect.kind,x
    jsr table_call
TC_SLOT null_tick
TC_SLOT slide_up_tick
TC_SLOT slide_down_tick
TC_SLOT portamento_tick
TC_SLOT vibrato_tick
TC_SLOT arpeggio_tick
TC_SLOT volume_slide_tick
TC_SLOT tremolo_tick
TC_SLOT cut_tick
TC_SLOT pulsemod_tick
.endp

; Command 0 is no effect.

.proc null_tick
    rts
.endp

;-------------------------[ Vibrato implementation ]--------------------------

.proc vibrato_tick
; check vibrato delay
    lda     mixer.tonals.effect.vibrato.counter,x
    beq     @@counter_ended
    dec     mixer.tonals.effect.vibrato.counter,x
    rts

    @@counter_ended:
; reset channel frequency
    lda     mixer.tonals.period_index,x
    jsr     set_period

; get sine value
    lda     mixer.tonals.effect.vibrato.pos,x
    and     #$1F
    tay
    lda     vibrato_table,y
    sta     value

; *** convert sine value to real delta freq, according to vibrato depth ***

    lda     #$00
    sta     temp.lo
    sta     temp.hi
    lda     mixer.tonals.effect.vibrato.param,x
    and     #$0F                    ; VibratoDepth in lower 4 bits
    tay
    clc
; this loop performs SineValue*VibratoDepth
  - lda     temp.lo
    adc     value
    sta     temp.lo
    bcc     +
    inc     temp.hi
    clc
  + dey
    bne     -

; this stores the result of (SineValue*VibratoDepth)/128 in temp.hi
    asl     temp.lo
    rol     temp.hi

    lda     mixer.tonals.effect.vibrato.pos,x
    and     #$3F
    cmp     #$20
    bcc     @@vib_add

    lda     mixer.tonals.period.lo,x
    sec
    sbc     temp.hi
    sta     mixer.tonals.period.lo,x
    bcs     @@vib_done
    dec     mixer.tonals.period.hi,x
    bcc     @@vib_done

    @@vib_add:
    lda     mixer.tonals.period.lo,x
    clc
    adc     temp.hi
    sta     mixer.tonals.period.lo,x
    bcc     @@vib_done
    inc     mixer.tonals.period.hi,x

    @@vib_done:
    ; increment vibrato pos
    lda     mixer.tonals.effect.vibrato.param,x
    lsr
    lsr
    lsr
    lsr
    clc
    adc     mixer.tonals.effect.vibrato.pos,x        ; add VibratoSpeed to VibratoPos
    sta     mixer.tonals.effect.vibrato.pos,x
    rts
.endp

; ProTracker sine table used for vibrato

vibrato_table:
.db $00,$18,$31,$4A,$61,$78,$8D,$A1
.db $B4,$C5,$D4,$E0,$EB,$F4,$FA,$FD
.db $FF,$FD,$FA,$F4,$EB,$E0,$D4,$C5
.db $B4,$A1,$8D,$78,$61,$4A,$31,$18

;----------------------[ Slide to note implementation ]-----------------------

.proc portamento_tick
    lda     mixer.tonals.effect.portamento.ctrl,x
    bpl     @@portamento_exit
    lsr
    bcc     @@portamento_down

    jsr     slide_up_tick
; check if slide frequency has been reached
    lda     mixer.tonals.period.lo,x
    cmp     mixer.tonals.effect.portamento.target.lo,x
    lda     mixer.tonals.period.hi,x
    sbc     mixer.tonals.effect.portamento.target.hi,x
    bpl     @@portamento_exit

    @@portamento_done:
    ; set final period
    lda     mixer.tonals.effect.portamento.target.lo,x
    sta     mixer.tonals.period.lo,x
    lda     mixer.tonals.effect.portamento.target.hi,x
    sta     mixer.tonals.period.hi,x
    ; halt
    lda     #$00
    sta     mixer.tonals.effect.portamento.ctrl,x
    rts

    @@portamento_down:
    jsr     slide_down_tick
; check if slide frequency has been reached
    lda     mixer.tonals.period.lo,x
    cmp     mixer.tonals.effect.portamento.target.lo,x
    lda     mixer.tonals.period.hi,x
    sbc     mixer.tonals.effect.portamento.target.hi,x
    bpl     @@portamento_done

    @@portamento_exit:
    rts
.endp

;-----------------------[ Slide down implementation ]-------------------------

.proc slide_down_tick
; slide down by adding slide amount to channel frequency
    lda     mixer.tonals.period.lo,x
    clc
    adc     mixer.tonals.effect.slide.amount,x
    sta     mixer.tonals.period.lo,x
    bcc     +
    inc     mixer.tonals.period.hi,x
  + rts
.endp

;------------------------[ Slide up implementation ]--------------------------

.proc slide_up_tick
; slide up by subtracting slide amount from channel frequency
    lda     mixer.tonals.period.lo,x
    sec
    sbc     mixer.tonals.effect.slide.amount,x
    sta     mixer.tonals.period.lo,x
    bcs     +
    dec     mixer.tonals.period.hi,x
  + rts
.endp

;------------------------[ Arpeggio implementation ]--------------------------

.proc arpeggio_tick
    lda     mixer.tonals.effect.arpeggio.pos,x
    lsr
    sec
  - cmp     #$03
    bcc     +
    sbc     #$03
    bpl     -

    ; A = Tick % 3
  + cmp     #$01
    beq     @@add_note1
    bcs     @@add_note2
    lda     mixer.tonals.period_index,x
    bpl     set_period

    @@add_note1:
    lda     mixer.tonals.effect.arpeggio.param,x
    lsr
    lsr
    lsr
    lsr
    clc
    adc     mixer.tonals.period_index,x
    bpl     set_period

    @@add_note2:
    lda     mixer.tonals.effect.arpeggio.param,x
    and     #$0F
    clc
    adc     mixer.tonals.period_index,x

    set_period:
    tay
    lda     period_table_lo,y
    sta     mixer.tonals.period.lo,x
    lda     period_table_hi,y
    sta     mixer.tonals.period.hi,x

    inc     mixer.tonals.effect.arpeggio.pos,x
    lda     mixer.tonals.effect.arpeggio.pos,x
    cmp     #6
    bcc     +
    lda     #0
    sta     mixer.tonals.effect.arpeggio.pos,x
  + rts
.endp

;------------------------[ Volume slide implementation ]--------------------------

.proc volume_slide_tick
    lda     mixer.tonals.effect.slide.amount,x
    and     #$F0
    bne     +
    ; slide down
    lda     mixer.envelopes.master,x
    sec
    sbc     mixer.tonals.effect.slide.amount,x
    bcs     ++
    lda     #0
 ++ sta     mixer.envelopes.master,x
    rts
    ; slide up
  + lda     mixer.tonals.effect.slide.amount,x
    lsr
    lsr
    lsr
    lsr
    clc
    adc     mixer.envelopes.master,x
    bcc ++
    lda     #$FF
 ++ sta     mixer.envelopes.master,x
    rts
.endp

;------------------------[ Tremolo implementation ]--------------------------

.proc tremolo_tick
; get sine value
    lda     mixer.tonals.effect.tremolo.pos,x
    and     #$1F
    tay
    lda     vibrato_table,y
    sta     value

    lda     #0
    sta     temp.lo
    sta     temp.hi
    lda     mixer.tonals.effect.tremolo.param,x
    and     #$0F                    ; TremoloDepth in lower 4 bits
    tay
    clc
; this loop performs SineValue*TremoloDepth
  - lda     temp.lo
    adc     value
    sta     temp.lo
    bcc     +
    inc     temp.hi
    clc
  + dey
    bne     -

; compute (SineValue*TremoloDepth)/16
    lsr     temp.hi
    ror     temp.lo
    lsr     temp.hi
    ror     temp.lo
    lsr     temp.hi
    ror     temp.lo
    lsr     temp.hi
    ror     temp.lo

    lda     mixer.tonals.effect.tremolo.pos,x
    and     #$3F
    cmp     #$20
    bcc     @@inc_vol

    lda     #$F0
    sec
    sbc     temp.lo
    bcs     +
    lda     #0
  + sta     mixer.envelopes.master,x
    jmp     @@inc_pos

    @@inc_vol:
    lda     #$F0
    clc
    adc     temp.lo
    bcc     +
    lda     #$FF
+   sta     mixer.envelopes.master,x

    @@inc_pos:
    lda     mixer.tonals.effect.tremolo.param,x ; speed in upper 4 bits
    lsr
    lsr
    lsr
    lsr
    clc
    adc     mixer.tonals.effect.tremolo.pos,x
    sta     mixer.tonals.effect.tremolo.pos,x
    rts
.endp

; ------ Cut note
; Reusing the vibrato struct because I'm lazy...
.proc cut_tick
    lda     mixer.tonals.effect.vibrato.pos,x
    cmp     mixer.tonals.effect.vibrato.param,x
    beq     @@cut
    bcs     +
    inc     mixer.tonals.effect.vibrato.pos,x
  + rts
    @@cut:
    lda     #0
    sta     mixer.tonals.period.lo,x
    sta     mixer.tonals.period.hi,x
    rts
.endp

; ------ Pulse width (duty cycle) modulation
; NB: can only be used on channels 0 and 1
; Reusing the vibrato struct because I'm lazy...
.proc pulsemod_tick
    lda    tracks.tick,x
    bne    +
    lda    #1
    sta    mixer.tonals.square.counter,x
    lda    mixer.tonals.effect.vibrato.param,x ; new duty cycle in lower 2 bits
    asl
    asl
    asl
    asl
    sta    mixer.tonals.square.duty_ctrl,x
  + rts
.endp

.end
