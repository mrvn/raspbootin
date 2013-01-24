/* mmio.h - access to MMIO registers */
/* Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>

namespace MMIO {
    // write to MMIO register
    static inline void write(uint32_t reg, uint32_t data) {
	uint32_t *ptr = (uint32_t*)reg;
	asm volatile("str %[data], [%[reg]]"
		     : : [reg]"r"(ptr), [data]"r"(data));
    }

    // read from MMIO register
    static inline uint32_t read(uint32_t reg) {
	uint32_t *ptr = (uint32_t*)reg;
	uint32_t data;
	asm volatile("ldr %[data], [%[reg]]"
		     : [data]"=r"(data) : [reg]"r"(ptr));
	return data;
    }
}

#endif // #ifndef MMIO_H
