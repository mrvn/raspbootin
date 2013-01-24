/* main.cc - the entry point for the kernel */
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

#include <stdint.h>
#include <uart.h>

extern "C" {
    // kernel_main gets called from boot.S. Declaring it extern "C" avoid
    // having to deal with the C++ name mangling.
    void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags);
}


#define LOADER_ADDR 0x2000000

const char hello[] = "\r\nRaspbootin V1.0\r\n";
const char halting[] = "\r\n*** system halting ***";

typedef void (*entry_fn)(uint32_t r0, uint32_t r1, uint32_t atags);

// kernel main function, it all begins here
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    UART::init();
again:
    UART::puts(hello);

    // request kernel by sending 3 breaks
    UART::puts("\x03\x03\x03");

    // get kernel size
    uint32_t size = UART::getc();
    size |= UART::getc() << 8;
    size |= UART::getc() << 16;
    size |= UART::getc() << 24;

    if (0x8000 + size > LOADER_ADDR) {
	UART::puts("SE");
	goto again;
    } else {
	UART::puts("OK");
    }
    
    // get kernel
    uint8_t *kernel = (uint8_t*)0x8000;
    while(size-- > 0) {
	*kernel++ = UART::getc();
    }

    // Kernel is loaded at 0x8000, call it via function pointer
    UART::puts("booting...");
    entry_fn fn = (entry_fn)0x8000;
    fn(r0, r1, atags);

    // fn() should never return. But it might, so make sure we catch it.
    // Wait a bit
    for(volatile int i = 0; i < 10000000; ++i) { }

    // Say goodbye and return to boot.S to halt.
    UART::puts(halting);
}

