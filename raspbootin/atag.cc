/* atag.cc - ARM bootloader tags (Header::print_all function) */
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

#include <atag.h>

void Header::print_all() const {
    for(const Header *current = this; current; current = current->next()) {
	kprintf("[%p] ", current);
	switch(current->tag) {
	case NONE: kprintf("NONE\n"); break;
	case CORE: ((const Core*)current)->print(); break;
	case MEM: ((const Mem*)current)->print(); break;
	case VIDEOTEXT: ((const Videotext*)current)->print(); break;
	case RAMDISK: ((const Ramdisk*)current)->print(); break;
	case INITRD2: ((const Initrd2*)current)->print(); break;
	case SERIAL: ((const Serial*)current)->print(); break;
	case REVISION: ((const Revision*)current)->print(); break;
	case VIDEOLFB: ((const VideoLFB*)current)->print(); break;
	case CMDLINE: ((const Cmdline*)current)->print(); break;
	}
    }
}

