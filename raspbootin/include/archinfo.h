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

#ifndef RASPBOOTIN_ARCHINFO_H
#define RASPBOOTIN_ARCHINFO_H

#include <stdint.h>
#include <atag.h>

class ArchInfo {
public:
    enum Archs { RPI, RPIplus, RPI2, NUM_ARCH_INFOS };
    constexpr ArchInfo(const char *model_, uint32_t peripherals_base_, int disk_led_gpio_,
	     bool disk_led_active_low_)
	: model(model_), peripherals_base(peripherals_base_),
	  disk_led_gpio(disk_led_gpio_),
	  disk_led_active_low(disk_led_active_low_) { }
    const char *model;
    const uint32_t peripherals_base;
    const int disk_led_gpio;
    const bool disk_led_active_low;
};

extern const ArchInfo *arch_info;

#endif // #ifndef RASPBOOTIN_ARCHINFO_H
