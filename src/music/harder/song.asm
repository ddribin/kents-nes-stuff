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

.include "song.inc"

song_instrument_table:
.dw env0 : .db $00,$00,$00,$40 : .db $00,$00 ; 0 square
.dw env0 : .db $00,$00,$00,$80 : .db $00,$00 ; 1 square
.dw env1 : .db $00,$00,$00,$00 : .db $00,$00 ; 2 square
.dw env1 : .db $00,$00,$00,$00 : .db $00,$00 ; 3 triangle
.dw env2 : .db $00,$00,$00,$00 : .db $00,$00 ; 4 triangle (short)
.dw env3 : .db $00,$02,$60,$00 : .db $00,$00 ; 5 triangle (bass drum)
.dw env4 : .db $00,$02,$20,$00 : .db $00,$00 ; 6 triangle (snare drum)
.dw env5 : .db $00,$00,$00,$00 : .db $00,$00 ; 7 noise (closed hihat)
.dw env6 : .db $00,$00,$00,$00 : .db $00,$00 ; 8 noise (open hihat)
.dw env7 : .db $00,$00,$00,$00 : .db $00,$00 ; 9 noise (snare)
.dw env8 : .db $00,$00,$00,$00 : .db $00,$00 ; 10 noise (cymbalo)
.dw env9 : .db $00,$00,$00,$00 : .db $00,$00 ; 11 noise (static)

env0:
.db $0C
.db $00,$00,$FF,$0C
.db $0C,$00,$00,$00
.db $FF,$FF
env1:
.db $0F
.db $00,$00,$FF,$0F
.db $0F,$00,$00,$00
.db $FF,$FF
env2:
.db $0F
.db $00,$00,$0E,$0F
.db $0F,$00,$00,$00
.db $FF,$FF
env3:
.db $0F
.db $00,$00,$05,$0F
.db $0F,$00,$00,$00
.db $FF,$FF
env4:
.db $0F
.db $00,$00,$08,$0F
.db $0F,$00,$00,$00
.db $FF,$FF
env5:
.db $0E
.db $01,$C0,$00,$00
.db $FF,$FF
env6:
.db $0D
.db $00,$B0,$00,$00
.db $FF,$FF
env7:
.db $0D
.db $01,$00,$00,$00
.db $FF,$FF
env8:
.db $0D
.db $00,$C0,$00,$00
.db $FF,$FF
env9:
.db $07
.db $FF,$FF

.end
