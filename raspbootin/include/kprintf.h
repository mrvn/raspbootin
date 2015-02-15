/* kprintf.h - kernel printf implementation
 * Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef RASPBOOTIN_KPRINTF_H
#define RASPBOOTIN_KPRINTF_H

#include <stddef.h>
#include <stdarg.h>
#include <sys/cdefs.h>

__BEGIN_DECLS
#define __PRINTFLIKE(__fmt,__varargs) __attribute__((__format__ (__printf__, __fmt, __varargs)))

void kprintf(const char *format, ...) __PRINTFLIKE(1, 2);

int snprintf(char *buf, size_t size, const char *format, ...) __PRINTFLIKE(3, 4);
int vsnprintf(char *buf, size_t size, const char *format, va_list args);

typedef void (*vcprintf_callback_t)(void *state, char c);

void cprintf(vcprintf_callback_t callback, void *state, const char* format,
	     ...) __PRINTFLIKE(3, 4);

void vcprintf(vcprintf_callback_t callback, void *state, const char* format,
	      va_list args);
__END_DECLS

#endif // #ifndef RASPBOOTIN_KPRINTF_H
