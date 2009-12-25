;
;    Copyright (C) 2009 Kent Hansen.
;
;    This program is free software; you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation; either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

.codeseg

.extrn start_song:proc
.extrn update_sound:proc

NSF_VERSION .equ 1
SONG_COUNT .equ 1
START_SONG .equ 1
SONG_NAME .equ "Satellite one (NES remake)"
ARTIST_NAME .equ "Original by Purple Motion"
COPYRIGHT .equ "Remake by SnowBro, 2008"

.db "NESM"
.db $1A
.db NSF_VERSION
.db SONG_COUNT
.db START_SONG
.dw $C000
.dw start_song
.dw update_sound
@@l1:
.db SONG_NAME
.dsb 32-($-@@l1)
@@l2:
.db ARTIST_NAME
.dsb 32-($-@@l2)
@@l3:
.db COPYRIGHT
.dsb 32-($-@@l3)
.dw $411A ; NTSC speed
.db 0,0,0,0,0,0,0,0 ; bank switch init values (not used)
.dw 0 ; PAL speed (not used)
.db 0 ; PAL/NTSC bits
.db 0 ; extra sound chip support
.db 0,0,0,0 ; unused

.end
