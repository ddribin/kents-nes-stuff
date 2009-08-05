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

; Sequencer track definitions.

.ifndef TRACK_H
.define TRACK_H

.include <common/ptr.h>

.struc order_state
pos         .byte               ; Position in order table
loop_pos    .byte               ; Order loop position
loop_count  .byte               ; Order loop count
.ends

.struc pattern_state
ptr         .ptr                ; Pointer to pattern
pos         .byte               ; Pattern position (byte offset)
loop_count  .byte               ; Pattern loop count
row         .byte               ; Row in pattern
row_count   .byte               ; Number of rows in pattern
row_status  .byte               ; on/off bits
transpose   .byte               ; Note transpose
.ends

; Structure that describes a sequencer track's state.
.struc track_state
speed               .byte       ; Number of ticks (frames) per row inc
tick                .byte       ; Tick in row
order               .order_state
pattern             .pattern_state
.ends

.endif  ; !TRACK_H
