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
; The mixer is responsible for outputting proper audio.

.include "mixer.h"
.include "track.h"

.dataseg

.public mixer   .mixer_state

.codeseg

.if sizeof tonal_state != sizeof envelope_state
.error "tonal_state and envelope_state must have the same size"
.endif
.if sizeof tonal_state != sizeof track_state
.error "tonal_state and track_state must have the same size"
.endif

;.define NO_SFX
;.define NO_MUTABLE_CHANNELS

.public mixer_tick
.public mixer_reset
.ifndef NO_MUTABLE_CHANNELS
.public mixer_get_muted_channels
.public mixer_set_muted_channels
.endif
.public mixer_get_master_vol
.public mixer_set_master_vol

.extrn envelope_tick:proc
.extrn effect_tick:proc
.extrn sfx_tick:proc
.extrn volume_table:byte
.extrn sound_status:byte

; Executes one "tick" of the mixer.
; This involves updating volume envelopes, music effects (vibrato, for example),
; and so forth, and most importantly, writing proper values to the audio
; hardware registers.
; The mixer handles updating of sound effects, and selects the proper "audio
; source" (sound effect or music) depending on whether a sound effect is playing
; or not.

.proc mixer_tick
    lda sound_status
    and #$20 ; paused?
    bne @@skip

; update volume envelopes
    ldx #3*sizeof envelope_state
  - jsr envelope_tick
    txa
    sec
    sbc #sizeof envelope_state
    tax
    bpl -

; update tonal effects
    ldx #3*sizeof tonal_state
  - jsr effect_tick
    txa
    sec
    sbc #sizeof tonal_state
    tax
    bpl -

; update square duties
; ### consider making this a plain effect instead
    ldx #1*sizeof tonal_state
  - lda mixer.tonals.square.counter,x
    beq +
    dec mixer.tonals.square.counter,x
    bne +
    lda mixer.tonals.square.duty_ctrl,x
    asl
    asl
    and #$C0
    sta mixer.tonals.square.duty,x ; set the new duty
  + txa
    sec
    sbc #sizeof tonal_state
    tax
    bpl -

    @@skip:
; write to NES audio regs

; channel 0
.ifndef NO_SFX
    lda mixer.sfx[0].ptr.hi
    beq +

    ; sfx
    ldx #0*sizeof sfx_state
    jsr sfx_tick
    jmp ++

    ; music
  +
.endif
.ifndef NO_MUTABLE_CHANNELS
    lda sound_status
    lsr
.endif
    lda mixer.envelopes[0].master
.ifndef NO_MUTABLE_CHANNELS
    bcc +
    lda #0 ; the channel is muted
  +
.endif
    and #$F0
    ora mixer.envelopes[0].vol.int
    tay
    lda volume_table,y
    ora mixer.master_vol
    tay
    lda volume_table,y
    ora mixer.tonals[0].square.duty
    ora #$30
    sta $4000
    lda #0
    sta $4001
    lda mixer.tonals[0].period.lo
    sta $4002
    lda mixer.tonals[0].period.hi
    cmp mixer.tonals[0].square.period_save
    beq ++
    sta $4003
    sta mixer.tonals[0].square.period_save

; channel 1
 ++ 
.ifndef NO_SFX
    lda mixer.sfx[1].ptr.hi
    beq +

    ; sfx
    ldx #1*sizeof sfx_state
    jsr sfx_tick
    jmp ++

    ; music
  +
.endif
.ifndef NO_MUTABLE_CHANNELS
    lda sound_status
    lsr
    lsr
.endif
    lda mixer.envelopes[1].master
.ifndef NO_MUTABLE_CHANNELS
    bcc +
    lda #0 ; the channel is muted
  +
.endif
    and #$F0
    ora mixer.envelopes[1].vol.int
    tay
    lda volume_table,y
    ora mixer.master_vol
    tay
    lda volume_table,y
    ora mixer.tonals[1].square.duty
    ora #$30
    sta $4004
    lda #0
    sta $4005
    lda mixer.tonals[1].period.lo
    sta $4006
    lda mixer.tonals[1].period.hi
    cmp mixer.tonals[1].square.period_save
    beq ++
    sta $4007
    sta mixer.tonals[1].square.period_save
 
; channel 2
 ++
.ifndef NO_SFX
    lda mixer.sfx[2].ptr.hi
    beq +

    ; sfx
    ldx #2*sizeof sfx_state
    jsr sfx_tick
    jmp ++

    ; music
  +
.endif
.ifndef NO_MUTABLE_CHANNELS
    lda sound_status
    lsr
    lsr
    lsr
.endif
    lda mixer.envelopes[2].master
.ifndef NO_MUTABLE_CHANNELS
    bcc +
    lda #0 ; the channel is muted
  +
.endif
    and #$F0
    ora mixer.envelopes[2].vol.int
    tay
    lda volume_table,y
    ora mixer.master_vol
    tay
    lda volume_table,y
    beq +
    ora #$FF
  + sta $4008
    lda mixer.tonals[2].period.lo
    sta $400A
    lda mixer.tonals[2].period.hi
    sta $400B

; channel 3
.ifndef NO_SFX
 ++ lda mixer.sfx[3].ptr.hi
    beq +

    ; sfx
    ldx #3*sizeof sfx_state
    jsr sfx_tick
    rts

    ; music
  +
.endif
.ifndef NO_MUTABLE_CHANNELS
    lda sound_status
    lsr
    lsr
    lsr
    lsr
.endif
    lda mixer.envelopes[3].master
.ifndef NO_MUTABLE_CHANNELS
    bcc +
    lda #0 ; the channel is muted
  +
.endif
    and #$F0
    ora mixer.envelopes[3].vol.int
    tay
    lda volume_table,y
    ora mixer.master_vol
    tay
    lda volume_table,y
    ora #$30
    sta $400C
    lda mixer.tonals[3].period.lo
    asl
    lda mixer.tonals[3].period.hi
    rol ; period / 128
    ora mixer.tonals[3].square.duty_ctrl ; bit 7 = RNG mode
    sta $400E
    lda #$08
    sta $400F
    rts
.endp

; Resets the mixer.
; It sets important fields of the mixer data structures so that things will
; behave correctly on invocations to mixer_tick().

.proc mixer_reset
    lda #$F0
    sta mixer.master_vol
; zap volume envelopes
    ldx #3*sizeof envelope_state
  - lda #0
    sta mixer.envelopes.phase,x
    sta mixer.envelopes.vol.int,x
    lda #$F0
    sta mixer.envelopes.master,x
    txa
    sec
    sbc #sizeof envelope_state
    tax
    bpl -
; zap tonals
    ldx #3*sizeof effect_state
  - lda #0
    sta mixer.tonals.effect.kind,x
    sta mixer.tonals.period_index,x
    sta mixer.tonals.period.lo,x
    sta mixer.tonals.period.hi,x
    txa
    sec
    sbc #sizeof effect_state
    tax
    bpl -
.ifndef NO_SFX
; zap sound effects
    ldx #3*sizeof sfx_state
  - lda #0
    sta mixer.sfx.ptr.hi,x
    txa
    sec
    sbc #sizeof sfx_state
    tax
    bpl -
.endif
; custom channel init
    lda #$80
    sta mixer.tonals[0].square.period_save
    sta mixer.tonals[1].square.period_save
; enable sound hardware channels
    lda #$0F
    sta $4015
    rts
.endp

.ifndef NO_MUTABLE_CHANNELS
.proc mixer_get_muted_channels
    lda sound_status
    and #$1F
    rts
.endp

.proc mixer_set_muted_channels
    pha
    lda sound_status
    and #$E0
    sta sound_status
    pla
    and #$1F
    ora sound_status
    sta sound_status
    rts
.endp
.endif

.proc mixer_get_master_vol
    lda mixer.master_vol
    rts
.endp

.proc mixer_set_master_vol
    sta mixer.master_vol
    rts
.endp

.end
