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
; Basic sprite stuff.

.include "sprite.h"

.dataseg

; variables used to implement "sprite shuffling" from frame to frame
.public sprite_index    .byte
.public sprite_base     .byte

; sprite memory
.public sprites .sprite_state[64]

; must be page-aligned since we use sprite DMA
.align sprites 256

.codeseg

.public reset_sprites
.public next_sprite_index

; Sets Y coordinate of all sprites to outside screen
; and resets the sprite index.
; Params:  None
; Returns: Nothing
.proc reset_sprites
    lda #$F4
    ldx #0
  - sta sprites._y,x
    inx
    inx
    inx
    inx
    bne -
    lda sprite_base
    sta sprite_index
    clc
    adc #SPRITE_BASE_INCR
    sta sprite_base
    rts
.endp

.proc next_sprite_index
    lda     sprite_index
    clc
    adc     #SPRITE_INDEX_INCR
    sta     sprite_index
    rts
.endp

.end
