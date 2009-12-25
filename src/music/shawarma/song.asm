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
.dw env1 : .db $00,$00,$00,$92 : .db $00,$00 ; 0
.dw env2 : .db $00,$00,$00,$22 : .db $00,$00 ; 1
.dw env0 : .db $00,$00,$00,$42 : .db $00,$00 ; 2
.dw env7 : .db $00,$00,$00,$00 : .db $00,$00 ; 3 triangle (short)
.dw env0 : .db $00,$00,$00,$00 : .db $00,$00 ; 4 triangle (infinite)
.dw env3 : .db $00,$00,$00,$00 : .db $00,$00 ; 5 noise (closed)
.dw env4 : .db $00,$00,$00,$00 : .db $00,$00 ; 6 noise (open)
.dw env5 : .db $00,$00,$00,$00 : .db $00,$00 ; 7 noise (snare)
.dw env6 : .db $00,$00,$00,$00 : .db $00,$00 ; 8 noise (cymbalo)
.dw env8 : .db $00,$00,$00,$80 : .db $00,$00 ; 9 noise (weird)
.dw env9 : .db $00,$00,$00,$22 : .db $00,$00 ; 10 square (short)

env0:
.db $08
.db $00,$00,$FF,$08
.db $08,$00,$00,$00
.db $FF,$FF
env1:
.db $08
.db $00,$00,$FF,$08
.db $08,$00,$00,$00
.db $FF,$FF
env2:
.db $08
.db $00,$00,$FF,$08
.db $08,$00,$00,$00
.db $FF,$FF
env3:
.db $08
.db $01,$80,$00,$00
.db $FF,$FF
env4:
.db $09
.db $00,$90,$00,$00
.db $FF,$FF
env5:
.db $08
.db $00,$70,$00,$00
.db $FF,$FF
env6:
.db $0A
.db $00,$40,$00,$00
.db $FF,$FF
env7:
.db $06
.db $01,$00,$00,$00
.db $FF,$FF
env8:
.db $04
.db $01,$00,$00,$00
.db $FF,$FF
env9:
.db $08
.db $01,$80,$FF,$02
.db $02,$00,$00,$00
.db $FF,$FF

.include "song.inc"

.end
