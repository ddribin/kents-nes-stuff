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

.dataseg

.codeseg

.extrn song_song:label

.public init
.public test_cycle

.public song_table

.extrn fill_all_nametables:proc
.extrn write_ppu_data_at:proc
.extrn screen_on:proc
.extrn start_song:proc
.extrn mixer_get_muted_channels:proc
.extrn mixer_set_muted_channels:proc
.extrn test_song:byte
.extrn test_song2:byte
.extrn square_test_song:byte
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
.incbin "packnam.dat"
.db $22,$A6,19
.char "SWEET CHILD O' MINE"
.db $22,$E3,25
.char "ORIGINAL BY GUNS N' ROSES"
.db $23,$24,23
.char "REMAKE BY SNOWBRO, 2008"
.db $23,$69,14
.char "MADE IN NORWAY"
.db $3F,$00,$20
.db $0F,$06,$08,$27
.db $0F,$06,$08,$27
.db $0F,$06,$08,$27
.db $0F,$06,$08,$27
.db $0F,$06,$08,$27
.db $0F,$06,$08,$27
.db $0F,$06,$08,$27
.db $0F,$06,$08,$27
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

song_table:
.dw 0
.dw song_song

.end
