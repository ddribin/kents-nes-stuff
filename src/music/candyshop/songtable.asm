;
;    This file is part of "Kent's Candy Shop Remake" (KCSR).
;
;    KCSR is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    KCSR is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with KCSR.  If not, see <http://www.gnu.org/licenses/>.
;

.codeseg

.public song_table

.extrn candy_song:label

song_table:
.dw 0
.dw candy_song

.end
