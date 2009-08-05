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

.codeseg

.public volume_table

; Volume table used for scaling channel volume according to master volume.

volume_table:
.db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00      ; MasterVol = 0
.db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$01      ; 1
.db $00,$00,$00,$00,$00,$00,$00,$00,$01,$01,$01,$01,$01,$01,$01,$02      ; 2
.db $00,$00,$00,$00,$00,$01,$01,$01,$01,$01,$02,$02,$02,$02,$02,$03      ; 3
.db $00,$00,$00,$00,$01,$01,$01,$01,$02,$02,$02,$02,$03,$03,$03,$04      ; ..
.db $00,$00,$00,$01,$01,$01,$02,$02,$02,$03,$03,$03,$04,$04,$04,$05
.db $00,$00,$00,$01,$01,$02,$02,$02,$03,$03,$04,$04,$04,$05,$05,$06
.db $00,$00,$00,$01,$01,$02,$02,$03,$03,$04,$04,$05,$05,$06,$06,$07
.db $00,$00,$01,$01,$02,$02,$03,$03,$04,$04,$05,$05,$06,$06,$07,$08
.db $00,$00,$01,$01,$02,$03,$03,$04,$04,$05,$06,$06,$07,$07,$08,$09
.db $00,$00,$01,$02,$02,$03,$04,$04,$05,$06,$06,$07,$08,$08,$09,$0A
.db $00,$00,$01,$02,$02,$03,$04,$05,$05,$06,$07,$08,$08,$09,$0A,$0B
.db $00,$00,$01,$02,$03,$04,$04,$05,$06,$07,$08,$08,$09,$0A,$0B,$0C
.db $00,$00,$01,$02,$03,$04,$05,$06,$06,$07,$08,$09,$0A,$0B,$0C,$0D
.db $00,$00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0A,$0B,$0C,$0D,$0E
.db $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0A,$0B,$0C,$0D,$0E,$0F

.end
