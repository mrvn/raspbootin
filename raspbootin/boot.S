/* boot.S - assembly startup code */
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
	
// To keep this in the first portion of the binary.
.section ".text.boot"

// Make Start global.
.globl Start

// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main
Start:
	// Setup the stack.
	mov	sp, #0x8000

	// we're loaded at 0x8000, relocate to _start.
.relocate:
	// copy from r3 to r4.
	mov	r3, #0x8000
	ldr	r4, =_start
	ldr	r9, =_data_end
1:
	// Load multiple from r3, and store at r4.
	ldmia	r3!, {r5-r8}
	stmia	r4!, {r5-r8}

	// If we're still below file_end, loop.
	cmp	r4, r9
	blo	1b

	// Clear out bss.
	ldr	r4, =_bss_start
	ldr	r9, =_bss_end
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
1:
	// store multiple at r4.
	stmia	r4!, {r5-r8}

	// If we're still below bss_end, loop.
	cmp	r4, r9
	blo	1b

	// Call kernel_main
	ldr	r3, =kernel_main
	blx	r3

	// halt
halt:
	wfe
	b	halt
