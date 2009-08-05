.ifndef LDC_H
.define LDC_H

.macro ldcay arg
    lda #<arg
    ldy #>arg
.endm

.endif
