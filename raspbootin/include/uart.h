/* uart.h - UART initialization & communication */
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

#ifndef UART_H
#define UART_H

#include <stdint.h>

namespace UART {
    /*
     * Initialize UART0.
     */
    void init(void);

    /*
     * Transmit a byte via UART0.
     * uint8_t Byte: byte to send.
     */
    void putc(uint8_t byte);

    /*
     * Receive a byte via UART0.
     *
     * Returns:
     * uint8_t: byte received.
     */
    uint8_t getc(void);

    /*
     * print a string to the UART one character at a time
     * const char *str: 0-terminated string
     */
    void puts(const char *str);
}

#endif // #ifndef UART_H
