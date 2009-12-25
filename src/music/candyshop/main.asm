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

.public init
.public test_cycle

.extrn fill_all_nametables:proc
.extrn write_ppu_data_at:proc
.extrn screen_on:proc
.extrn start_song:proc
.extrn mixer_get_muted_channels:proc
.extrn mixer_set_muted_channels:proc
.extrn joypad0_posedge:byte

.proc init
    lda #0
    jsr fill_all_nametables
    lda #<@@ppu_data
    ldy #>@@ppu_data
    jsr write_ppu_data_at
    lda #1
    jsr start_song
    jsr screen_on
    rts
@@ppu_data:
.charmap "font.tbl"
.incbin "candy.dat"
.db $22,$CB,10
.char "CANDY SHOP"
.db $22,$E7,19
.char "ORIGINAL BY MADONNA"
.db $23,$25,23
.char "REMAKE BY SNOWBRO, 2009"
.db $23,$49,14
.char "MADE IN NORWAY"
.db $3F,$00,$20
.db $0F,$06,$26,$20
.db $0F,$00,$10,$20
.db $0F,$00,$10,$20
.db $0F,$00,$10,$20
.db $0F,$00,$10,$20
.db $0F,$00,$10,$20
.db $0F,$00,$10,$20
.db $0F,$00,$10,$20
.db 0
.endp

.proc test_cycle
    lda joypad0_posedge
    beq +
    jsr mixer_get_muted_channels
    eor joypad0_posedge
    and #$0F
    jsr mixer_set_muted_channels
  + rts
.endp

.end
