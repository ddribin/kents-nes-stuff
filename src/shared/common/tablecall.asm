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
; Jump table entrypoint.

.include "ptr.h"

.dataseg zeropage

addr_table .ptr

.codeseg

.public table_call

; Calls one in an array of procedures.
; A: Routine # to execute
; A is used as an index into a table of code addresses.
; The jump table itself MUST be located directly after the JSR to this
; routine, so that its address can be popped from the stack.
.proc table_call
    asl
    tay
    iny     ; b/c stack holds jump table address MINUS 1
    pla     ; low address of table
    sta addr_table.lo
    pla     ; high address of table
    sta addr_table.hi
    lda [addr_table],y
    pha
    iny
    lda [addr_table],y
    pha
    rts     ; jump to address
.endp

.end
