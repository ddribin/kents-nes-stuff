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

; Description:
; Period table for square and triangle channels.

.codeseg

.public period_table_lo
.public period_table_hi

period_table_lo:
.db                                     $F1,$7F,$13 ; A-1 .. B-1
.db $AE,$4E,$F3,$9E,$4D,$01,$B9,$75,$35,$F8,$BF,$89 ; C-2 .. B-2
.db $57,$27,$F9,$CF,$A6,$80,$5C,$3A,$1A,$FC,$DF,$C4 ; C-3 .. B-3
.db $AB,$93,$7C,$67,$53,$40,$2E,$1D,$0D,$FE,$EF,$E2 ; C-4 .. B-4
.db $D5,$C9,$BE,$B3,$A9,$A0,$97,$8E,$86,$7F,$79,$71 ; C-5 .. B-5
.db $6A,$64,$5F,$59,$54,$50,$4B,$47,$43,$3F,$3B,$38 ; C-6 .. B-6
.db $35,$32,$2F,$2C,$2A,$28,$25,$23,$21,$1F,$1D,$1C ; C-7 .. B-7

period_table_hi:
.db                                     $07,$07,$07
.db $06,$06,$05,$05,$05,$05,$04,$04,$04,$03,$03,$03
.db $03,$03,$02,$02,$02,$02,$02,$02,$02,$01,$01,$01
.db $01,$01,$01,$01,$01,$01,$01,$01,$01,$00,$00,$00
.db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
.db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
.db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00

.end
