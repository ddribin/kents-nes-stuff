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

.ifndef MMC3_H
.define MMC3_H

MMC3_CTRL_REG           .equ    $8000
MMC3_PAGE_REG           .equ    $8001
MMC3_MIRROR_REG         .equ    $A000
MMC3_SRAM_REG           .equ    $A001
MMC3_IRQ_COUNTER_REG    .equ    $C000
MMC3_IRQ_LATCH_REG      .equ    $C001
MMC3_IRQ_DISABLE_REG    .equ    $E000
MMC3_IRQ_ENABLE_REG     .equ    $E001

MMC3_MIRROR_V   .equ    0
MMC3_MIRROR_H   .equ    1

MMC3_SRAM_DISABLE   .equ 0
MMC3_SRAM_ENABLE    .equ 1

.endif  ; !MMC3_H
