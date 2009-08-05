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
; MMC3-specific data and code.

.include "mmc3.h"

.dataseg

.public irq_count   .byte

; The CHR bank indices that are set in NMI.
.public chr_banks   .byte[6]

current_bank    .byte

.codeseg

.public reset_bank
.public set_bank
.public swap_bank

; Load the 16K bank in current_bank
    reset_bank:
    lda current_bank
; Set and load 16K bank in A
    set_bank:
    sta current_bank
; Load 16K bank in A, but keep old current_bank value
    swap_bank:
    asl
    ldy #$06    ; command = swap 1st 8K bank
    sty MMC3_CTRL_REG
    sta MMC3_PAGE_REG
    iny         ; command = swap 2nd 8K bank
    ora #1
    sty MMC3_CTRL_REG
    sta MMC3_PAGE_REG
    rts

.end
