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

.ifndef SPRITE_H
.define SPRITE_H

; Hardware regs.
SPRITE_ADDR_REG     .equ     $2003
SPRITE_IO_REG       .equ     $2004
SPRITE_DMA_REG      .equ     $4014

; Record that describes a sprite's attributes.
.record sprite_attribs v_flip:1, h_flip:1, pri:1, pad0:3, pal:2

SPRITE_ATTR_V_FLIP  .equ mask sprite_attribs::v_flip
SPRITE_ATTR_H_FLIP  .equ mask sprite_attribs::h_flip
SPRITE_ATTR_PRI     .equ mask sprite_attribs::pri
SPRITE_ATTR_PAL     .equ mask sprite_attribs::pal

; Structure that describes a sprite.
; The field order is such that it can be DMA'ed directly to sprite RAM.
.struc sprite_state
_y      .byte
tile    .byte
attr    .sprite_attribs
_x      .byte
.ends

SPRITE_INDEX_INCR .equ 15*4
SPRITE_BASE_INCR  .equ 17*4

.endif  ; !SPRITE_H
