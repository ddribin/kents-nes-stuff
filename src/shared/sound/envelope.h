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

.ifndef ENVELOPE_H
.define ENVELOPE_H

.include <common/ptr.h>
.include <common/fixedpoint.h>

; Structure that describes a volume envelope's state.
.struc envelope_state
phase   .byte
ptr     .ptr    ; Pointer to envelope data
pos     .byte   ; Position in data
vol     .fp_8_8 ; Current volume
step    .fp_8_8 ; Volume increment
dest    .byte   ; Destination volume
hold    .byte   ; Hold length at destination
master  .byte
padding .byte[2] ; to get same size as track_state
.ends

; Flags for envelope_state.phase
ENV_RESET   .equ    $80
ENV_PROCESS .equ    $40
ENV_SUSTAIN .equ    $20

.endif  ; !ENVELOPE_H
