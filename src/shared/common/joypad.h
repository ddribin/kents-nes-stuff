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

.ifndef JOYPAD_H
.define JOYPAD_H

JOYPAD0_IO_REG   .equ $4016
JOYPAD1_IO_REG   .equ $4017

.record joypad _a:1, _b:1, select:1, start:1, up:1, down:1, left:1, right:1

JOYPAD_BUTTON_A         .equ mask joypad::_a
JOYPAD_BUTTON_B         .equ mask joypad::_b
JOYPAD_BUTTON_SELECT    .equ mask joypad::select
JOYPAD_BUTTON_START     .equ mask joypad::start
JOYPAD_BUTTON_UP        .equ mask joypad::up
JOYPAD_BUTTON_DOWN      .equ mask joypad::down
JOYPAD_BUTTON_LEFT      .equ mask joypad::left
JOYPAD_BUTTON_RIGHT     .equ mask joypad::right

.endif  ; !JOYPAD_H
