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

.ifndef FIXEDPOINT_H
.define FIXEDPOINT_H

.include "int16.h"

; 8.8 fixed-point.
.struc fp_8_8
int     .byte   ; integer part
frac    .byte   ; fractional part
.ends

; 16.8 fixed-point.
.struc fp_16_8
int     .int16  ; integer part
frac    .byte   ; fractional part
.ends

.endif  ; !FIXEDPOINT_H
