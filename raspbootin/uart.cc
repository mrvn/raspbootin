/* uart.cc - UART initialization & communication */
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

/* Reference material:
 * http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 * Chapter 13: UART
 */

#include <stdint.h>
#include <mmio.h>
#include <uart.h>

namespace UART {
    enum {
	// The GPIO registers base address.
	GPIO_OFFSET = 0x00200000,

	// The offsets for reach register.

	// Controls actuation of pull up/down to ALL GPIO pins.
	GPPUD = (GPIO_OFFSET + 0x94),

	// Controls actuation of pull up/down for specific GPIO pin.
	GPPUDCLK0 = (GPIO_OFFSET + 0x98),

	// The base address for UART.
	UART0_OFFSET = GPIO_OFFSET + 0x00001000,

	// The offsets for reach register for the UART.
	UART0_DR     = (UART0_OFFSET + 0x00),
	UART0_RSRECR = (UART0_OFFSET + 0x04),
	UART0_FR     = (UART0_OFFSET + 0x18),
	UART0_ILPR   = (UART0_OFFSET + 0x20),
	UART0_IBRD   = (UART0_OFFSET + 0x24),
	UART0_FBRD   = (UART0_OFFSET + 0x28),
	UART0_LCRH   = (UART0_OFFSET + 0x2C),
	UART0_CR     = (UART0_OFFSET + 0x30),
	UART0_IFLS   = (UART0_OFFSET + 0x34),
	UART0_IMSC   = (UART0_OFFSET + 0x38),
	UART0_RIS    = (UART0_OFFSET + 0x3C),
	UART0_MIS    = (UART0_OFFSET + 0x40),
	UART0_ICR    = (UART0_OFFSET + 0x44),
	UART0_DMACR  = (UART0_OFFSET + 0x48),
	UART0_ITCR   = (UART0_OFFSET + 0x80),
	UART0_ITIP   = (UART0_OFFSET + 0x84),
	UART0_ITOP   = (UART0_OFFSET + 0x88),
	UART0_TDR    = (UART0_OFFSET + 0x8C),
    };

    /*
     * delay function
     * int32_t delay: number of cycles to delay
     *
     * This just loops <delay> times in a way that the compiler
     * wont optimize away.
     */
    void delay(int32_t count) {
	asm volatile("1: subs %[count], %[count], #1; bne 1b"
		     : : [count]"r"(count));
    }
    
    /*
     * Initialize UART0.
     */
    void init(void) {
	// Disable UART0.
	MMIO::write(UART0_CR, 0x00000000);
	// Setup the GPIO pin 14 && 15.
    
	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	MMIO::write(GPPUD, 0x00000000);
	delay(150);

	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	MMIO::write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);

	// Write 0 to GPPUDCLK0 to make it take effect.
	MMIO::write(GPPUDCLK0, 0x00000000);
    
	// Clear pending interrupts.
	MMIO::write(UART0_ICR, 0x7FF);

	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// UART_CLOCK = 3000000; Baud = 115200.

	// Divider = 3000000/(16 * 115200) = 1.627 = ~1.
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	MMIO::write(UART0_IBRD, 1);
	MMIO::write(UART0_FBRD, 40);

	// Enable FIFO & 8 bit data transmissio (1 stop bit, no parity).
	MMIO::write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

	// Mask all interrupts.
	MMIO::write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) |
		    (1 << 6) | (1 << 7) | (1 << 8) |
		    (1 << 9) | (1 << 10));

	// Enable UART0, receive & transfer part of UART.
	MMIO::write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
    }

    /*
     * Transmit a byte via UART0.
     * uint8_t Byte: byte to send.
     */
    void putc(uint8_t byte) {
	// wait for UART to become ready to transmit
	while(true) {
	    if (!(MMIO::read(UART0_FR) & (1 << 5))) {
		break;
	    }
	}
	MMIO::write(UART0_DR, byte);
    }

    /*
     * Receive a byte via UART0.
     *
     * Returns:
     * uint8_t: byte received.
     */
    uint8_t getc(void) {
	// wait for UART to have recieved something
	while(true) {
	    if (!(MMIO::read(UART0_FR) & (1 << 4))) {
		break;
	    }
	}
	return MMIO::read(UART0_DR);
    }

    /*
     * print a string to the UART one character at a time
     * const char *str: 0-terminated string
     */
    void puts(const char *str) {
	while(*str) {
	    UART::putc(*str++);
	}
    }
}
