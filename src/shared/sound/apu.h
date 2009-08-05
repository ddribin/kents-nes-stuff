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

.ifndef APU_H
.define APU_H

; Hardware regs.
APU_SQUARE0_BASE_REG    .equ $4000
APU_SQUARE1_BASE_REG    .equ $4004
APU_TRI_BASE_REG        .equ $4008
APU_NOISE_BASE_REG      .equ $400C
APU_DMC_BASE_REG        .equ $4010
APU_STATUS_REG          .equ $4015  ; read
APU_CTRL_REG            .equ $4015  ; write
APU_FRAME_CTRL_REG      .equ $4017

.record apu_frame_ctrl mode:1, irq:1, pad0:6

APU_FRAME_CTRL_MODE     .equ mask apu_frame_ctrl::mode
APU_FRAME_CTRL_IRQ      .equ mask apu_frame_ctrl::irq

.record apu_status dmc_irq:1, frame_irq:1, pad0:1, dmc_ctr:1, noise_ctr:1, tri_ctr:1, sq1_ctr:1, sq0_ctr:1
.record apu_ctrl pad0:3, dmc_on:1, noise_on:1, tri_on:1, sq1_on:1, sq0_on:1

; Bitmasks to interpret APU_STATUS_REG.
APU_STATUS_DMC_IRQ      .equ mask apu_status::dmc_irq
APU_STATUS_FRAME_IRQ    .equ mask apu_status::frame_irq
APU_STATUS_DMC_CTR      .equ mask apu_status::dmc_ctr
APU_STATUS_NOISE_CTR    .equ mask apu_status::noise_ctr
APU_STATUS_TRI_CTR      .equ mask apu_status::tri_ctr
APU_STATUS_SQUARE1_CTR  .equ mask apu_status::sq1_ctr
APU_STATUS_SQUARE0_CTR  .equ mask apu_status::sq0_ctr

; Bitmasks that can be OR'ed together to create APU_CTRL_REG-compatible value.
APU_CTRL_DMC_ON         .equ APU_STATUS_DMC_CTR
APU_CTRL_NOISE_ON       .equ APU_STATUS_NOISE_CTR
APU_CTRL_TRI_ON         .equ APU_STATUS_TRI_CTR
APU_CTRL_SQUARE1_ON     .equ APU_STATUS_SQ1_CTR
APU_CTRL_SQUARE0_ON     .equ APU_STATUS_SQ0_CTR

.record square_ctrl duty:2, loop:1, env:1, vol:4
.record square_sweep enable:1, period:3, neg:1, shift:3

APU_SQUARE_CTRL_DUTY    .equ mask square_ctrl::duty ; Duty cycle
APU_SQUARE_CTRL_LOOP    .equ mask square_ctrl::loop ; Loop envelope
APU_SQUARE_CTRL_HALT    .equ mask square_ctrl::loop ; Halt length (NB: shared with loop)
APU_SQUARE_CTRL_ENV     .equ mask square_ctrl::env  ; Disable envelope
APU_SQUARE_CTRL_VOL     .equ mask square_ctrl::vol  ; Volume / envelope period

APU_SQUARE_SWEEP_ENABLE .equ mask square_sweep::enable
APU_SQUARE_SWEEP_PERIOD .equ mask square_sweep::period
APU_SQUARE_SWEEP_NEG    .equ mask square_sweep::neg
APU_SQUARE_SWEEP_SHIFT  .equ mask square_sweep::shift

.record noise_ctrl pad0:2, loop:1, env:1, vol:4
.record noise_reg2 short:1, pad0:3, period_idx:4
.record noise_reg3 length_idx:5, pad0:3

APU_NOISE_CTRL_LOOP .equ mask noise_ctrl::loop  ; Loop enable
APU_NOISE_CTRL_ENV  .equ mask noise_ctrl::env   ; Envelope disable
APU_NOISE_CTRL_VOL  .equ mask noise_ctrl::vol   ; Volume / envelope period

.record dmc_ctrl irq:1, loop:1, pad0:2, freq_idx:4

APU_DMC_CTRL_IRQ            .equ mask dmc_ctrl::irq         ; IRQ enable
APU_DMC_CTRL_LOOP           .equ mask dmc_ctrl::loop        ; Loop
APU_DMC_CTRL_FREQ_INDEX     .equ mask dmc_ctrl::freq_idx    ; Frequency index

.endif  ; !APU_H
