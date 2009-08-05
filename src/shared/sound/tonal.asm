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
; Handles pattern commands for channels 0,1,2,3.

.include <common/tablecall.h>
.include "mixer.h"

.codeseg

.public process_tonal_pattern_byte

.extrn mixer:mixer_state
.extrn table_call:proc
.extrn fetch_pattern_byte:proc
.extrn set_track_speed:proc
.extrn set_all_tracks_speed:proc
.extrn period_table_lo:byte
.extrn period_table_hi:byte
.extrn instrument_table:ptr

; Processes one byte received on channel.
; Params:   A = byte
;           X = offset of channel data

.proc process_tonal_pattern_byte
    ora     #0
    bpl     is_note           ; if bit 7 clear, it's a note #

    cmp     #$E0              ; is it a command?
    bcc     is_append
    cmp     #$F0
    and     #$0F
    bcs     is_command
    ; set effect + param
    sta     mixer.tonals.effect.kind,x
    beq     +
    jsr     fetch_pattern_byte
    sta     mixer.tonals.effect.slide.amount,x      ; this is a union
    lda     #0
    lda     mixer.tonals.effect.vibrato.delay,x
    sta     mixer.tonals.effect.vibrato.counter,x
    sta     mixer.tonals.effect.vibrato.pos,x
  + sec
    rts

    is_command:
    jsr     go_command
    sec
    rts

    is_append:
    and     #$7F                    ; note # in lower 7 bits
    bpl     set_note             ; skip the volume & vibrato retrig

    is_note:
    pha
    lda     #$80
    sta     mixer.tonals.square.period_save,x ; this is a union
    lda     mixer.envelopes.master,x
    lsr     ; CF=1 if the volume has been overridden by a previous volume command
    bcs     +
    lda     #$78
  + asl
    sta     mixer.envelopes.master,x
    lda     #ENV_RESET
    sta     mixer.envelopes.phase,x             ; volume envelope phase = init
    lda     #0
    sta     mixer.tonals.effect.vibrato.pos,x        ; reset vibrato position
    lda     mixer.tonals.effect.vibrato.delay,x
    sta     mixer.tonals.effect.vibrato.counter,x          ; reset vibrato delay
    pla

    set_note:
    ldy     mixer.tonals.effect.kind,x
    cpy     #PORTAMENTO_EFFECT  ; if slide parameter present...
    beq     init_slide          ; ... slide from old to new note
; no slide, set new period immediately
    sta     mixer.tonals.period_index,x
    tay
    lda     period_table_lo,y
    sta     mixer.tonals.period.lo,x
    lda     period_table_hi,y
    sta     mixer.tonals.period.hi,x
; channel-specific init
; ### consider making a plain effect
    lda     mixer.tonals.square.duty_ctrl,x
    pha
    and     #$C0
    sta     mixer.tonals.square.duty,x
    pla
    and     #$0F
    sta     mixer.tonals.square.counter,x
;
    clc
    rts

    init_slide:
    cmp     mixer.tonals.period_index,x              ; CF = slide direction (0=down,1=up)
    sta     mixer.tonals.period_index,x
    tay
    lda     period_table_lo,y
    sta     mixer.tonals.effect.portamento.target.lo,x
    lda     period_table_hi,y
    sta     mixer.tonals.effect.portamento.target.hi,x
;
    lda     #$40
    rol     ; bit 0 = slide direction
    sta     mixer.tonals.effect.portamento.ctrl,x
    rts
.endp

; Processes a channel command.

.proc go_command
    jsr     table_call
TC_SLOT set_instr
TC_SLOT release
TC_SLOT set_mastervol
TC_SLOT set_speed
TC_SLOT end_row
.endp

; Sets instrument.

.proc set_instr
    jsr     fetch_pattern_byte
    sta     mixer.tonals.instrument,x
    asl
    asl
    asl                     ; each instrument is 8 bytes long
    tay
    lda     [instrument_table],y
    sta     mixer.envelopes.ptr.lo,x
    iny
    lda     [instrument_table],y
    sta     mixer.envelopes.ptr.hi,x
    iny
    lda     [instrument_table],y
    sta     mixer.tonals.effect.vibrato.delay,x
    iny
    lda     [instrument_table],y
    sta     mixer.tonals.effect.kind,x
    iny
    lda     [instrument_table],y
    sta     mixer.tonals.effect.slide.amount,x
    iny
    lda     [instrument_table],y
    sta     mixer.tonals.square.duty_ctrl,x ; this is a union
    rts
.endp

; Disables volume envelope looping.

.proc release
    lda     #1
    sta     mixer.envelopes.hold,x
    rts
.endp

; Sets the envelope master volume.

.proc set_mastervol
    jsr     fetch_pattern_byte
    ora     #1 ; indicates that volume was explicitly set
    sta     mixer.envelopes.master,x
    rts
.endp

.proc set_speed
    jsr     fetch_pattern_byte
; ### this is buggy
.if 0
    jsr     set_track_speed
.else
    jsr     set_all_tracks_speed
.endif
    rts
.endp

; this command is used when there is no note for the row, only commands
.proc end_row
    lda     mixer.envelopes.master,x
    and     #$FE
    sta     mixer.envelopes.master,x
    pla
    pla
    clc
    rts
.endp

.end
