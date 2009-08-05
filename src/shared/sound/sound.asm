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
; Top-level sound stuff.
; Call start_song to start playing a song.

.include <common/ptr.h>

.dataseg

.public current_song .byte
; bits 4..0: whether channel is muted (1=yes)
; bit 5: paused (1=yes)
.public sound_status .byte   

.codeseg

.public start_song
.public maybe_start_song
.public update_sound
.public pause_music
.public unpause_music

.extrn sequencer_tick:proc
.extrn sequencer_load:proc
.extrn mixer_tick:proc
.extrn mixer_reset:proc

; The song table must be defined by each program that uses the sound
; engine. It must be a table of pointers to the songs that can be
; played.
.extrn song_table:ptr

; Updates sound. Call once a frame.

.proc update_sound
    lda sound_status
    and #$20 ; paused?
    bne +
    jsr sequencer_tick
  + jmp mixer_tick
.endp

; Starts a song.
; Params:   A = song #

.proc start_song
    sta current_song
    asl
    tay
    lda song_table.lo,y
    pha
    lda song_table.hi,y
    tay
    pla
    jsr sequencer_load
    jmp mixer_reset
.endp

.proc pause_music
    lda #$20
    ora sound_status
    sta sound_status
    rts
.endp

.proc unpause_music
    lda sound_status
    and #$DF ; zero bit 5
    sta sound_status
    rts
.endp

; Starts song only if it's different from current song.
; Params:   A = song #

.proc maybe_start_song
    cmp current_song
    beq +
    jmp start_song
  + rts
.endp

.end
