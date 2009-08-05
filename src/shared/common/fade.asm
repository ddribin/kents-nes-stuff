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
; Routines for fading in/out the palette, from/to black/white.
; Recipe for use:
; 1. Load the palette: see load_palette in palette.asm.
; 2. Set the range of palette entries to fade (set_fade_range),
; set the delay (set_fade_delay).
; 3. call one of the start_fade_* functions.

.include "tablecall.h"

.dataseg

; private variables
temp_palette .byte[32]
counter      .byte
fade_type    .byte
pal_index_lo .byte
pal_index_hi .byte
frames_per_step .byte

.codeseg

; exported API
.public set_fade_range
.public set_fade_delay
.public start_fade_from_black
.public start_fade_from_white
.public start_fade_to_black
.public start_fade_to_white
.public fade_out_step
.public fade_in_step

; external symbols
.extrn palette:byte
.extrn ppu_buffer:byte
.extrn start_palette_ppu_string:proc
.extrn end_ppu_string:proc
.extrn table_call:proc
.extrn start_timer:proc
.extrn set_timer_callback:proc

; Sets the range of palette entries affected by fade routines.
; Params:  A, Y = range
.proc set_fade_range
    sta pal_index_lo
    sty pal_index_hi
    rts
.endp

; Sets the # of frames per step during fading.
; Params:  A = # of frames per step
.proc set_fade_delay
    sta frames_per_step
    rts
.endp

.proc fade_out_step
    jsr     palette_to_temp_palette
    jsr     fade_to_black_step
    jsr     temp_palette_to_ppu_buffer
    rts
.endp

.proc fade_in_step
    jsr     palette_to_temp_palette
    jsr     fade_from_black_step
    jsr     temp_palette_to_ppu_buffer
    rts
.endp

;-----------------------------------------------------------------------------

; Does custom initialization for the selected fade type.
; Params: None
    fade_init:
    jsr     palette_to_temp_palette
    lda     fade_type
    jsr     table_call
TC_SLOT fade_from_black_init
TC_SLOT fade_from_white_init
TC_SLOT fade_to_black_init
TC_SLOT fade_to_white_init

    fade_from_black_init:
    ldy     pal_index_hi
  - lda     temp_palette,y
    and     #$0F            ; color intensity = 0
    sta     temp_palette,y
    dey
    bmi     +
    cpy     pal_index_lo
    bcs     -
  + rts

    fade_from_white_init:
    ldy     pal_index_hi
  - lda     temp_palette,y
    ora     #$30            ; color intensity = full
    cmp     #$3F
    bne     +
    lda     #$30
  + sta     temp_palette,y
    dey
    cpy     pal_index_lo
    bpl     -
    rts

    fade_to_black_init:
    rts

    fade_to_white_init:
    ldy     pal_index_hi
  - lda     palette,y
    cmp     #$0F
    bne     +
    lda     #$F0
  + sta     temp_palette,y
    dey
    cpy     pal_index_lo
    bpl     -
    rts

;-----------------------------------------------------------------------------

; Executes one step of fading for the selected fade type.
    fade_step:
    lda     counter
    beq     +
    jsr     go_fade_handler
    jsr     temp_palette_to_ppu_buffer
    dec     counter
  + rts

    go_fade_handler:
    lda     fade_type
    jsr     table_call
TC_SLOT fade_from_black_step
TC_SLOT fade_from_white_step
TC_SLOT fade_to_black_step
TC_SLOT fade_to_white_step

    fade_from_black_step:
    ldy     pal_index_hi
  - lda     temp_palette,y
    cmp     palette,y       ; has color reached full intensity?
    bcs     +               ; if yes, don't modify it
    adc     #$10            ; increase color intensity by 1
    sta     temp_palette,y
  + dey
    bmi     +
    cpy     pal_index_lo
    bcs     -
  + rts

    fade_from_white_step:
    ldy     pal_index_hi
  - lda     temp_palette,y
    cmp     palette,y       ; has color reached full intensity?
    beq     ++              ; if yes, don't modify it
    cmp     #$00
    bne     +
    lda     #$0F
    bne     ++
  + sec
    sbc     #$10            ; decrease color intensity by 1
    sta     temp_palette,y
 ++ dey
    cpy     pal_index_lo
    bpl     -
    rts

    fade_to_black_step:
    ldy     pal_index_hi
  - lda     temp_palette,y
    sec
    sbc     #$10            ; decrease color intensity by 1
    bcs     +               ; if result is < 0...
    lda     #$0F            ; ... color = black
  + sta     temp_palette,y
    dey
    cpy     pal_index_lo
    bpl     -
    rts

    fade_to_white_step:
    ldy     pal_index_hi
  - lda     temp_palette,y
    clc
    adc     #$10            ; increase color intensity by 1
    cmp     #$40
    bcc     +               ; if result is >= 64...
    lda     #$30            ; ... color = white
  + sta     temp_palette,y
    dey
    cpy     pal_index_lo
    bpl     -
    rts

;-----------------------------------------------------------------------------

    fade_timer_callback:
    jsr     fade_step
    bne     start_fade_timer
; fade done
    rts

    start_fade_timer:
    lda     #1
    ldy     frames_per_step
    jsr     start_timer
    lda     #<fade_timer_callback
    ldy     #>fade_timer_callback
    jmp     set_timer_callback

;-----------------------------------------------------------------------------

    start_fade_from_black:
    lda     #0
    beq     start_fade

    start_fade_from_white:
    lda     #1
    bne     start_fade

    start_fade_to_black:
    lda     #2
    bne     start_fade

    start_fade_to_white:
    lda     #3

    start_fade:
    sta     fade_type
    jsr     fade_init
    jsr     temp_palette_to_ppu_buffer
    lda     #4
    sta     counter
    jmp     start_fade_timer

;-----------------------------------------------------------------------------

; Writes the intermediate palette to PPU buffer.
    temp_palette_to_ppu_buffer:
    lda     pal_index_hi
    sec
    sbc     pal_index_lo
    adc     #0
    tax
    lda     pal_index_lo
    jsr     start_palette_ppu_string
    ldy     pal_index_lo
  - lda     temp_palette,y
    iny
    sta     ppu_buffer,x
    inx
    cpy     pal_index_hi
    bcc     -
    beq     -
    jmp     end_ppu_string

; Copies palette to temp palette.
    palette_to_temp_palette:
    ldy     pal_index_hi
  - lda     palette,y
    sta     temp_palette,y
    dey
    bmi     +
    cpy     pal_index_lo
    bcs     -
  + rts

.end
