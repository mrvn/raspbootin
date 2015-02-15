/* atag.h - ARM bootloader tags */
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

#ifndef RASPBOOTIN_ATAG_H
#define RASPBOOTIN_ATAG_H

#include <stddef.h>
#include <stdint.h>
#include <new>
#include <kprintf.h>

/*********************************************************************
 * Static - a class that can't be dynamicall allocated               *
 ********************************************************************/
class Static {
public:
    Static() { }
    ~Static() { }
    void* operator new (std::size_t /* size */, void* ptr) throw() {
        // placement new is allowed
        return ptr;
    }
private:
    void* operator new (std::size_t size) throw (std::bad_alloc) = delete;
    void* operator new (std::size_t size, const std::nothrow_t& nothrow_value) throw() = delete;
};

/*********************************************************************
 * Original - a class that can't be copied                           *
 ********************************************************************/
class Original {
public:
    ~Original() { }
    enum Guard { ALLOW };
protected:
    Original(Guard) { // no default constructor to prevent accidental calls
    }
private:
    Original(const Original&) = delete;
    Original operator =(const Original&) = delete;
};

enum Tag {NONE = 0x00000000, CORE = 0x54410001,
	  MEM = 0x54410002, VIDEOTEXT = 0x54410003,
	  RAMDISK = 0x54410004, INITRD2 = 0x54420005,
	  SERIAL = 0x54410006, REVISION = 0x54410007,
	  VIDEOLFB = 0x54410008, CMDLINE = 0x54410009,
};

class Header : public Original, public Static {
public:
    template<typename T>
    const T * find() const {
	const Header * p = this;
	while(p->tag != NONE) {
	    if (p->tag == T::TAG) {
		return (T*)p;
	    }
	    p = p->next();
	}
	return NULL;
    }
    template<typename T>
    const T * next() const {
	const Header * h = next();
	if (h == NULL) return NULL;
	return h->find<T>();
    }
    void print_all() const;
    void print() const {
	kprintf("Unknown tag tag = %lu, tag_size = %lu\n",
		tag, tag_size);
    }
    uint32_t tag_size;
    uint32_t tag;
protected:
    const Header * next() const {
	if (tag == NONE) {
	    return NULL;
	}
	return (const Header *)(((uint32_t*)this) + tag_size);
    }
};
    
class Core : public Header {
public:
    const Core * next() const { return Header::next<Core>(); }
    void print() const {
	kprintf("Core: writable = %s, pagesize = %#010lx, rootdev = %#010lx\n",
		writable ? "true" : "false", pagesize, rootdev);
    }
    struct {
	uint32_t writable:1;
    };
    uint32_t pagesize;
    uint32_t rootdev;
    static const uint32_t TAG = CORE;
};

class Mem : public Header {
public:
    const Mem * next() const { return Header::next<Mem>(); }
    void print() const {
	kprintf("Mem: start = %#010lx, size = %#010lx\n", start, size);
    }
    uint32_t size;
    uint32_t start;
    static const uint32_t TAG = MEM;
};

class Videotext : public Header {
public:
    const Videotext * next() const {
	return Header::next<Videotext>();
    }
    void print() const {
	kprintf("Videotext: x = %hhu, y = %hhu, page = %hu, mode = %hhu, cols = %hhu, ega_bx = %hu, lines = %hhu, isvga = %hhu, points = %hu\n",
		x, y, video_page, video_mode, video_cols, video_ega_bx,
		video_lines, video_isvga, video_points);
    }
    uint8_t  x;
    uint8_t  y;
    uint16_t video_page;
    uint8_t  video_mode;
    uint8_t  video_cols;
    uint16_t video_ega_bx;
    uint8_t  video_lines;
    uint8_t  video_isvga;
    uint16_t video_points;
    static const uint32_t TAG = VIDEOTEXT;
};

class Ramdisk : public Header {
public:
    const Ramdisk * next() const { return Header::next<Ramdisk>(); }
    void print() const {
	kprintf("Ramdisk: load = %s, prompt = %s, size = %#010lx, start = %#010lx\n",
		load ? "true" : "false", prompt ? "true" : "false",
		size, start);
    }
    struct {
	uint32_t load:1;
	uint32_t prompt:1;
    };
    uint32_t size;
    uint32_t start;
    static const uint32_t TAG = RAMDISK;
};

class Initrd2 : public Header {
public:
    const Initrd2 * next() const { return Header::next<Initrd2>(); }
    void print() const {
	kprintf("Initrd2: start = %010lx, size = %#010lx\n", start, size);
    }
    uint32_t start;
    uint32_t size;
    static const uint32_t TAG = INITRD2;
};

class Serial : public Header {
public:
    const Serial * next() const { return Header::next<Serial>(); }
    void print() const {
	kprintf("Serial: low = %lu, high = %lu\n", low, high);
    }
    uint32_t low;
    uint32_t high;
    static const uint32_t TAG = SERIAL;
};

class Revision : public Header {
public:
    const Revision * next() const { return Header::next<Revision>(); }
    void print() const {
	kprintf("Revision: rev = %#010lx\n", rev);
    }
    uint32_t rev;
    static const uint32_t TAG = REVISION;
};

class VideoLFB : public Header {
public:
    const VideoLFB * next() const { return Header::next<VideoLFB>(); }
    void print() const {
	kprintf("VideoLFB: width = %hu, height = %hu, depth = %hu, linelength = %hu, base = %#010lx, size = %#lx, red_size = %hhu, red_pos = %hhu, green_size = %hhu, green_pos = %hhu, blue_size = %hhu, blue_pos = %hhu, rsvd_size = %hhu, rsvd_pos = %hhu\n",
		lfb_width, lfb_height, lfb_depth, lfb_linelength,
		lfb_base, lfb_size, red_size, red_pos, green_size,
		green_pos, blue_size, blue_pos, rsvd_size, rsvd_pos);
    }
    uint16_t lfb_width;
    uint16_t lfb_height;
    uint16_t lfb_depth;
    uint16_t lfb_linelength;
    uint32_t lfb_base;
    uint32_t lfb_size;
    uint8_t  red_size;
    uint8_t  red_pos;
    uint8_t  green_size;
    uint8_t  green_pos;
    uint8_t  blue_size;
    uint8_t  blue_pos;
    uint8_t  rsvd_size;
    uint8_t  rsvd_pos;
    static const uint32_t TAG = VIDEOLFB;
};

class Cmdline : public Header {
public:
    const Cmdline * next() const { return Header::next<Cmdline>(); }
    void print() const {
	kprintf("Cmdline: '%s'\n", cmdline);
    }
    char cmdline[1];
    static const uint32_t TAG = CMDLINE;
};

#endif // #ifndef RASPBOOTIN_ATAG_H
