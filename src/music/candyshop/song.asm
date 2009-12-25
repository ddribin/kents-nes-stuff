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

.public song_song

song_instrument_table:
.dw env0 : .db $00,$00,$00,$92 : .db $00,$00 ; 0
.dw env2 : .db $00,$00,$00,$23 : .db $00,$00 ; 1
.dw env0 : .db $00,$00,$00,$42 : .db $00,$00 ; 2
.dw env1 : .db $00,$00,$00,$00 : .db $00,$00 ; 3 triangle (short)
.dw env0 : .db $00,$00,$00,$00 : .db $00,$00 ; 4 triangle (infinite)
.dw env3 : .db $00,$00,$00,$00 : .db $00,$00 ; 5 noise (closed)
.dw env4 : .db $00,$00,$00,$00 : .db $00,$00 ; 6 noise (open)
.dw env5 : .db $00,$00,$00,$00 : .db $00,$00 ; 7 noise (snare)
.dw env1 : .db $00,$00,$00,$00 : .db $00,$00 ; 8 triangle (very short)
.dw env6 : .db $00,$00,$00,$92 : .db $00,$00 ; 9 short chorus vocals

env0:
.db $0F
.db $00,$00,$FF,$0F
.db $0F,$00,$00,$00
.db $FF,$FF
env1:
.db $0E
.db $03,$00,$00,$00
.db $FF,$FF
env3:
.db $0F
.db $04,$00,$00,$00
.db $FF,$FF
env4:
.db $0C
.db $00,$F0,$00,$00
.db $FF,$FF
env5:
.db $0F
.db $01,$C0,$00,$00
.db $FF,$FF
env6:
.db $0A
.db $00,$00,$03,$0A
.db $03,$00,$00,$00
.db $FF,$FF
env2:
.db $0B
.db $00,$00,$FF,$0B
.db $0B,$00,$00,$00
.db $FF,$FF

.include "song.inc"

.end
