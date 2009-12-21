/*
    This file is part of xm2nes.

    xm2nes is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xm2nes is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xm2nes.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef XM2NES_H
#define XM2NES_H

#include "xm.h"
#include "instrmap.h"

void convert_xm_to_nes(const struct xm *, int,
                       const struct instr_mapping *,
                       const char *, FILE *);

#endif
