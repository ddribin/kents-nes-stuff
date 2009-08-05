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
; Core IRQ handling.
; Call set_irq_handler() to set the function to be executed when an
; IRQ happens.

.include "ptr.h"

.dataseg

irq_handler .ptr

.codeseg

.public irq
.public set_irq_handler

; IRQ entrypoint.

.proc irq
    sei
    pha
    txa
    pha
    tya
    pha
    jsr go_irq_handler
    pla
    tay
    pla
    tax
    pla
    rti
.endp

; Dispatches the IRQ handler.

.proc go_irq_handler
    jmp [irq_handler]
.endp

; Sets the IRQ handler routine.
; Params: A, Y = address of handler

.proc set_irq_handler
    sta irq_handler.lo
    sty irq_handler.hi
    rts
.endp

.end
