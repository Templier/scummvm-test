/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "tucker/tucker.h"
#include "tucker/graphics.h"

namespace Tucker {

const uint8 TuckerEngine::_locationWidthTableGame[85] = {
	1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 4, 2, 1,
	1, 2, 1, 2, 4, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 4, 1,
	1, 1, 1, 1, 2, 1, 2, 2, 2, 4, 4, 2, 2, 1, 1, 1, 4, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
};

const uint8 TuckerEngine::_locationWidthTableDemo[70] = {
	1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 2, 1, 2, 4, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1,
	1, 1, 1, 1, 2, 1, 2, 2, 2, 4, 4, 2, 2, 1, 1, 0
};

const uint8 TuckerEngine::_sprA02LookupTable[88] = {
	 0,  6,  2,  8,  1,  0,  6,  0,  2,  2,  2,  1,  2,  0,  1,  1,
	 6,  0,  1,  2,  1,  2,  3,  0,  6, 12,  7,  7,  1,  8,  1,  0,
	 3,  0,  4,  5,  0,  0,  3,  3,  2,  7,  7,  0,  4,  1,  5,  2,
	 4,  1,  1,  2,  4,  3,  1,  0,  2,  3,  4,  1,  1,  5,  3,  3,
	 1,  5,  3,  0,  1,  0,  0,  2,  2,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0
};

const uint8 TuckerEngine::_sprC02LookupTable[100] = {
	 0,  0,  6, 20,  3,  3, 15,  5,  9,  6,  7,  8,  8,  6,  3,  6,
	13,  3,  4, 10,  0,  7,  2, 34, 14,  0,  2,  3,  8,  3,  3,  3,
	19, 13,  1,  0,  2,  3,  0,  0,  0,  5,  5, 12,  0,  1,  0,  1,
	 3,  6,  7,  6,  0,  7,  5,  1,  2,  6,  3,  4,  9, 18,  0, 12,
	 0,  2, 10,  0,  0, 19,  0,  2,  2,  1, 22,  0,  0,  0,  0,  3,
	 0,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  1,  0
};

const uint8 TuckerEngine::_sprC02LookupTable2[100] = {
	0, 0, 1, 3, 1, 2, 3, 2, 2, 1, 1, 5, 2, 1, 1, 3, 3, 1,
	1, 4, 0, 1, 1, 4, 4, 0, 2, 1, 3, 3, 3, 2, 4, 4, 1, 0,
	1, 3, 0, 0, 0, 1, 1, 7, 0, 1, 0, 1, 1, 1, 7, 3, 0, 2,
	1, 1, 1, 2, 1, 1, 2, 3, 0, 5, 0, 1, 5, 0, 1, 4, 0, 1,
	1, 1, 1, 0, 0, 0, 0, 1, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

const int TuckerEngine::_staticData3Table[1600] = {
	0x014, 0x014, 0x015, 0x016, 0x017, 0x017, 0x016, 0x015, 0x3E7, 0x0A9, 0x0A9, 0x0AA, 0x0AA, 0x0AB, 0x0AB, 0x0AC,
	0x0AC, 0x3E7, 0x05E, 0x05F, 0x060, 0x061, 0x3E7, 0x0AD, 0x0AE, 0x0AF, 0x0B0, 0x3E7, 0x052, 0x053, 0x054, 0x055,
	0x3E7, 0x056, 0x057, 0x058, 0x059, 0x3E7, 0x05A, 0x05B, 0x05C, 0x05D, 0x3E7, 0x062, 0x063, 0x064, 0x065, 0x3E7,
	0x066, 0x067, 0x068, 0x069, 0x3E7, 0x00C, 0x00D, 0x00E, 0x00F, 0x3E7, 0x01A, 0x01B, 0x01C, 0x01D, 0x3E7, 0x01E,
	0x01F, 0x020, 0x021, 0x3E7, 0x024, 0x025, 0x026, 0x027, 0x3E7, 0x0B1, 0x0B2, 0x0B3, 0x0B4, 0x3E7, 0x0CB, 0x0CC,
	0x0CD, 0x0CE, 0x3E7, 0x0CF, 0x0D0, 0x0D1, 0x0D2, 0x3E7, 0x0D3, 0x0D4, 0x0D5, 0x0D6, 0x3E7, 0x0D7, 0x0D8, 0x0D9,
	0x0DA, 0x3E7, 0x0B5, 0x0B6, 0x0B7, 0x0B8, 0x3E7, 0x04A, 0x04B, 0x04C, 0x04D, 0x3E7, 0x04E, 0x04F, 0x050, 0x051,
	0x3E7, 0x02A, 0x02B, 0x02C, 0x02D, 0x3E7, 0x02E, 0x02F, 0x030, 0x031, 0x3E6, 0x0E6, 0x001, 0x3E7, 0x0B9, 0x0BA,
	0x0BB, 0x0BC, 0x3E7, 0x06A, 0x06B, 0x06C, 0x06D, 0x3E7, 0x032, 0x033, 0x034, 0x035, 0x3E7, 0x036, 0x037, 0x038,
	0x039, 0x3E7, 0x03A, 0x03B, 0x03C, 0x03D, 0x3E7, 0x03E, 0x03F, 0x040, 0x041, 0x3E7, 0x042, 0x043, 0x044, 0x045,
	0x3E7, 0x046, 0x047, 0x048, 0x049, 0x3E7, 0x06E, 0x06F, 0x070, 0x071, 0x3E6, 0x045, 0x000, 0x3E7, 0x072, 0x073,
	0x074, 0x075, 0x3E7, 0x076, 0x077, 0x078, 0x079, 0x3E7, 0x07A, 0x07B, 0x07C, 0x07D, 0x3E7, 0x07F, 0x080, 0x081,
	0x082, 0x3E7, 0x085, 0x086, 0x087, 0x088, 0x3E7, 0x089, 0x08A, 0x08B, 0x08C, 0x3E7, 0x08F, 0x090, 0x091, 0x092,
	0x3E7, 0x0BD, 0x0BE, 0x0BF, 0x0C0, 0x3E7, 0x0C1, 0x0C2, 0x0C3, 0x0C4, 0x3E7, 0x0C5, 0x0C6, 0x0C7, 0x0C8, 0x3E7,
	0x093, 0x094, 0x095, 0x096, 0x3E7, 0x099, 0x099, 0x099, 0x09A, 0x09A, 0x09A, 0x09B, 0x09B, 0x09B, 0x09C, 0x09C,
	0x09C, 0x3E7, 0x09D, 0x09D, 0x09E, 0x09E, 0x09F, 0x09F, 0x0A0, 0x0A0, 0x3E7, 0x0A1, 0x0A1, 0x0A2, 0x0A2, 0x0A3,
	0x0A3, 0x0A4, 0x0A4, 0x3E7, 0x0A5, 0x0A5, 0x0A6, 0x0A6, 0x0A7, 0x0A7, 0x0A8, 0x0A8, 0x3E7, 0x0DE, 0x0DF, 0x0E0,
	0x0E1, 0x3E7, 0x010, 0x011, 0x012, 0x013, 0x3E7, 0x0E3, 0x0E4, 0x0E5, 0x0E6, 0x3E6, 0x03F, 0x000, 0x3E7, 0x000,
	0x001, 0x002, 0x003, 0x3E7, 0x004, 0x005, 0x006, 0x007, 0x3E7, 0x008, 0x009, 0x00A, 0x00B, 0x3E7, 0x00C, 0x00D,
	0x00E, 0x00F, 0x3E7, 0x010, 0x011, 0x012, 0x013, 0x3E7, 0x014, 0x015, 0x016, 0x017, 0x3E7, 0x018, 0x019, 0x01A,
	0x01B, 0x3E7, 0x01D, 0x01E, 0x01F, 0x020, 0x3E7, 0x021, 0x022, 0x023, 0x024, 0x3E7, 0x025, 0x026, 0x027, 0x028,
	0x3E7, 0x029, 0x02A, 0x02B, 0x02C, 0x3E7, 0x02D, 0x02E, 0x02F, 0x030, 0x3E7, 0x031, 0x032, 0x033, 0x034, 0x3E7,
	0x035, 0x036, 0x037, 0x038, 0x3E7, 0x039, 0x03A, 0x03B, 0x03C, 0x3E7, 0x03D, 0x03E, 0x03F, 0x040, 0x3E7, 0x041,
	0x042, 0x043, 0x044, 0x3E7, 0x049, 0x04A, 0x04B, 0x04C, 0x3E7, 0x04D, 0x04E, 0x04F, 0x050, 0x3E7, 0x051, 0x052,
	0x053, 0x054, 0x3E7, 0x055, 0x056, 0x057, 0x058, 0x3E7, 0x059, 0x05A, 0x05B, 0x05C, 0x3E7, 0x05D, 0x05E, 0x05F,
	0x060, 0x3E7, 0x061, 0x062, 0x063, 0x064, 0x3E7, 0x068, 0x069, 0x06A, 0x06B, 0x3E7, 0x06C, 0x06D, 0x06E, 0x06F,
	0x3E7, 0x070, 0x071, 0x072, 0x073, 0x3E7, 0x074, 0x075, 0x076, 0x077, 0x3E7, 0x07A, 0x07B, 0x07C, 0x07D, 0x3E7,
	0x07E, 0x07F, 0x080, 0x081, 0x3E7, 0x082, 0x083, 0x084, 0x085, 0x3E7, 0x086, 0x087, 0x088, 0x089, 0x3E7, 0x08A,
	0x08B, 0x08C, 0x08D, 0x3E7, 0x08E, 0x08E, 0x08F, 0x08F, 0x090, 0x090, 0x091, 0x091, 0x3E7, 0x000, 0x001, 0x002,
	0x003, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x3E7,
	0x004, 0x005, 0x006, 0x007, 0x3E7, 0x008, 0x009, 0x00A, 0x00B, 0x3E7, 0x00C, 0x00D, 0x00E, 0x00F, 0x3E7, 0x010,
	0x011, 0x012, 0x013, 0x3E7, 0x014, 0x015, 0x016, 0x017, 0x3E7, 0x018, 0x019, 0x01A, 0x01B, 0x01C, 0x01D, 0x01E,
	0x01F, 0x020, 0x021, 0x022, 0x023, 0x024, 0x025, 0x026, 0x027, 0x028, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E,
	0x02F, 0x030, 0x031, 0x032, 0x3E7, 0x033, 0x034, 0x035, 0x3E7, 0x036, 0x037, 0x038, 0x3E7, 0x039, 0x03A, 0x03B,
	0x3E6, 0x091, 0x003, 0x3E7, 0x03B, 0x03A, 0x039, 0x3E6, 0x091, 0x000, 0x3E7, 0x03C, 0x03D, 0x03E, 0x03F, 0x040,
	0x041, 0x042, 0x043, 0x044, 0x3E6, 0x06E, 0x000, 0x3E7, 0x045, 0x046, 0x047, 0x048, 0x049, 0x04A, 0x04B, 0x04C,
	0x04D, 0x04E, 0x3E6, 0x068, 0x002, 0x3E7, 0x04F, 0x050, 0x051, 0x052, 0x3E7, 0x052, 0x051, 0x050, 0x04F, 0x3E7,
	0x053, 0x054, 0x055, 0x056, 0x3E7, 0x057, 0x058, 0x059, 0x05A, 0x05B, 0x05C, 0x05D, 0x05E, 0x05F, 0x060, 0x061,
	0x062, 0x063, 0x064, 0x065, 0x3E7, 0x066, 0x067, 0x068, 0x069, 0x06A, 0x06B, 0x06C, 0x06D, 0x06E, 0x06F, 0x070,
	0x071, 0x072, 0x073, 0x074, 0x075, 0x076, 0x3E7, 0x077, 0x078, 0x079, 0x07A, 0x07B, 0x07C, 0x07D, 0x3E6, 0x069,
	0x001, 0x3E7, 0x07D, 0x07C, 0x07B, 0x07A, 0x079, 0x078, 0x077, 0x3E6, 0x069, 0x000, 0x3E7, 0x07E, 0x07F, 0x080,
	0x081, 0x082, 0x083, 0x084, 0x3E6, 0x06A, 0x001, 0x3E7, 0x084, 0x083, 0x082, 0x081, 0x080, 0x07F, 0x07E, 0x3E6,
	0x06A, 0x000, 0x3E7, 0x085, 0x086, 0x087, 0x088, 0x089, 0x08A, 0x08B, 0x08C, 0x08D, 0x08E, 0x08F, 0x090, 0x091,
	0x092, 0x093, 0x094, 0x095, 0x096, 0x097, 0x098, 0x099, 0x09A, 0x09B, 0x09C, 0x09D, 0x09E, 0x09F, 0x0A0, 0x0A1,
	0x0A2, 0x0A3, 0x0A4, 0x0A5, 0x0A6, 0x0A7, 0x0A8, 0x0A9, 0x3E7, 0x0AA, 0x0AB, 0x0AC, 0x0AD, 0x0AE, 0x0AF, 0x0B0,
	0x0B1, 0x0B2, 0x0B3, 0x3E6, 0x06B, 0x000, 0x3E7, 0x0B3, 0x3E7, 0x0B4, 0x0B4, 0x0B5, 0x0B5, 0x0B6, 0x0B6, 0x0B7,
	0x0B7, 0x3E7, 0x0B8, 0x0B9, 0x0B9, 0x0BB, 0x0BC, 0x0BD, 0x0BE, 0x0BF, 0x3E7, 0x0C0, 0x0C1, 0x0C2, 0x0C3, 0x0C4,
	0x0C5, 0x0C6, 0x0C7, 0x0C8, 0x0C9, 0x0CA, 0x0CB, 0x0CC, 0x0CD, 0x0CE, 0x0CF, 0x0D0, 0x0D1, 0x0D2, 0x0D3, 0x0D4,
	0x0D5, 0x0D6, 0x0D7, 0x0D8, 0x3E7, 0x0D9, 0x0DA, 0x0DB, 0x0DC, 0x0DD, 0x0DE, 0x0DF, 0x0E0, 0x0E1, 0x0E2, 0x0E3,
	0x0E4, 0x0E5, 0x0E6, 0x0E7, 0x0E8, 0x0E9, 0x3E7, 0x1C5, 0x0EA, 0x0EB, 0x0EC, 0x0ED, 0x0EE, 0x0EF, 0x0F0, 0x0F1,
	0x0F2, 0x3E7, 0x0F3, 0x0F4, 0x0F5, 0x0F6, 0x0F7, 0x0F8, 0x0F9, 0x0FA, 0x3E6, 0x074, 0x001, 0x3E7, 0x0FB, 0x0FC,
	0x0FD, 0x0FE, 0x3E7, 0x0FF, 0x100, 0x101, 0x102, 0x3E7, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A,
	0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x3E7, 0x116, 0x115, 0x114,
	0x113, 0x112, 0x111, 0x110, 0x10F, 0x10E, 0x3E7, 0x117, 0x118, 0x119, 0x11A, 0x3E7, 0x11B, 0x11C, 0x11D, 0x11E,
	0x3E7, 0x11F, 0x120, 0x121, 0x122, 0x3E7, 0x123, 0x124, 0x125, 0x3E6, 0x091, 0x003, 0x3E7, 0x125, 0x124, 0x123,
	0x3E7, 0x126, 0x127, 0x128, 0x3E7, 0x128, 0x127, 0x126, 0x3E7, 0x129, 0x12A, 0x12B, 0x12C, 0x12D, 0x12E, 0x12F,
	0x130, 0x131, 0x132, 0x133, 0x134, 0x135, 0x136, 0x137, 0x138, 0x139, 0x13A, 0x13B, 0x13C, 0x13D, 0x13E, 0x13F,
	0x140, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148, 0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E, 0x14F,
	0x150, 0x151, 0x152, 0x3E7, 0x153, 0x154, 0x155, 0x156, 0x157, 0x159, 0x15A, 0x15B, 0x15C, 0x3E7, 0x15E, 0x15F,
	0x160, 0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0x16B, 0x16C, 0x16D, 0x16E, 0x16F,
	0x170, 0x171, 0x172, 0x3E7, 0x173, 0x173, 0x173, 0x173, 0x173, 0x173, 0x173, 0x174, 0x174, 0x174, 0x174, 0x174,
	0x174, 0x3E7, 0x175, 0x175, 0x175, 0x175, 0x175, 0x175, 0x176, 0x3E7, 0x177, 0x178, 0x179, 0x17A, 0x17B, 0x17C,
	0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x3E7, 0x188, 0x189, 0x18A, 0x18B,
	0x18C, 0x3E7, 0x198, 0x199, 0x19A, 0x19B, 0x19C, 0x19D, 0x19E, 0x19F, 0x1A0, 0x1A1, 0x1A2, 0x1A3, 0x1A4, 0x1A5,
	0x1A6, 0x1A7, 0x1A8, 0x1A9, 0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1AE, 0x1AF, 0x3E7, 0x1B0, 0x1B1, 0x1B2, 0x3E7, 0x1B3,
	0x1B4, 0x1B5, 0x1B6, 0x1B7, 0x1B8, 0x1B9, 0x1BA, 0x3E7, 0x1BB, 0x1BC, 0x1BD, 0x1BE, 0x1BF, 0x1C0, 0x1C1, 0x1C2,
	0x1C3, 0x1C4, 0x3E6, 0x06F, 0x000, 0x3E7, 0x098, 0x099, 0x09A, 0x3E7, 0x09A, 0x099, 0x098, 0x3E7, 0x09D, 0x09E,
	0x09F, 0x3E7, 0x09F, 0x09E, 0x09D, 0x3E7, 0x0A1, 0x0A2, 0x0A3, 0x3E7, 0x0A6, 0x0A7, 0x0A8, 0x0A9, 0x0AA, 0x0AB,
	0x0AC, 0x0AD, 0x3E6, 0x08E, 0x002, 0x3E7, 0x0AE, 0x0AF, 0x0B0, 0x0B1, 0x0B2, 0x0B3, 0x0B4, 0x0B5, 0x0B6, 0x0B7,
	0x0B8, 0x0B9, 0x0BA, 0x0BB, 0x0BC, 0x0BD, 0x0BE, 0x3E7, 0x0F2, 0x0F2, 0x0F3, 0x0F3, 0x0F4, 0x0F4, 0x3E6, 0x030,
	0x002, 0x3E7, 0x1D3, 0x1D4, 0x1D5, 0x1D6, 0x1D7, 0x1D8, 0x3E7, 0x0F6, 0x0F7, 0x0F8, 0x0F8, 0x3E7, 0x0FB, 0x0FC,
	0x0FD, 0x0FE, 0x0FF, 0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x3E6, 0x07F, 0x002, 0x3E7, 0x106, 0x107, 0x108,
	0x109, 0x10A, 0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x3E7, 0x1E8, 0x1E7, 0x1E6, 0x1E5, 0x1E4, 0x1E3,
	0x1E2, 0x3E6, 0x095, 0x002, 0x3E7, 0x1A7, 0x1A8, 0x3E7, 0x1A9, 0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1A9, 0x1AA, 0x1AB,
	0x1AC, 0x3E6, 0x09D, 0x003, 0x3E7, 0x1A0, 0x1A1, 0x3E7, 0x0EA, 0x0EB, 0x0EC, 0x0ED, 0x0EE, 0x0EF, 0x0F0, 0x0F1,
	0x0F2, 0x0F3, 0x0F4, 0x0F5, 0x3E7, 0x0F6, 0x0F7, 0x0F8, 0x0F9, 0x0FA, 0x0FB, 0x0FC, 0x0FD, 0x0FE, 0x0FF, 0x100,
	0x101, 0x3E7, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A, 0x10B, 0x10C, 0x10D, 0x3E7, 0x10E,
	0x10F, 0x110, 0x111, 0x3E7, 0x112, 0x113, 0x114, 0x115, 0x3E7, 0x116, 0x117, 0x3E7, 0x118, 0x119, 0x11A, 0x11B,
	0x3E7, 0x11C, 0x11D, 0x3E7, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0x123, 0x124, 0x125, 0x126, 0x127, 0x128, 0x129,
	0x3E7, 0x12A, 0x12B, 0x12C, 0x12D, 0x12E, 0x12F, 0x130, 0x131, 0x132, 0x133, 0x134, 0x135, 0x3E7, 0x136, 0x137,
	0x138, 0x139, 0x13A, 0x13B, 0x13C, 0x13D, 0x13E, 0x13F, 0x140, 0x141, 0x3E7, 0x106, 0x107, 0x108, 0x109, 0x10A,
	0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x3E7, 0x1A2, 0x1A3, 0x1A4, 0x1A2, 0x1A3, 0x1A4, 0x1A2, 0x1A3,
	0x1A4, 0x1A2, 0x1A3, 0x1A4, 0x3E6, 0x09D, 0x005, 0x3E7, 0x3E7, 0x3E7, 0x3E7, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

uint8 TuckerEngine::_locationHeightTableGame[80] = {
	0x00, 0x1C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3C, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint8 TuckerEngine::_locationHeightTableDemo[70] = {
	0x00, 0x1C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3C, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int TuckerEngine::_objectKeysPosXTable[80] = {
	0x000, 0x0A0, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x12B, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x140, 0x000, 0x000, 0x000, 0x000, 0x09E, 0x060, 0x0C0, 0x040, 0x0A0, 0x12C, 0x068, 0x098,
	0x08E, 0x09A, 0x0A0, 0x098, 0x092, 0x096, 0x09A, 0x09A, 0x08C, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0A0, 0x000,
	0x086, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0A0, 0x185, 0x000, 0x0A0, 0x140, 0x140, 0x000,
	0x000, 0x124, 0x140, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
};

int TuckerEngine::_objectKeysPosYTable[80] = {
	0x000, 0x06B, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x080, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x075, 0x000, 0x000, 0x000, 0x000, 0x086, 0x02B, 0x079, 0x07C, 0x07C, 0x07B, 0x073, 0x07B,
	0x06C, 0x08A, 0x086, 0x086, 0x086, 0x086, 0x083, 0x083, 0x07B, 0x000, 0x000, 0x000, 0x000, 0x000, 0x078, 0x000,
	0x082, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x089, 0x08A, 0x000, 0x088, 0x082, 0x076, 0x000,
	0x000, 0x07F, 0x083, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
};

int TuckerEngine::_objectKeysLocationTable[80] = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x001, 0x000, 0x000, 0x000, 0x000, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
	0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x000, 0x000, 0x001, 0x001, 0x001, 0x000,
	0x000, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
};

int TuckerEngine::_mapSequenceFlagsLocationTable[70] = {
	0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	1, 1, 1, 1, 0, 0,
};

const uint8 TuckerEngine::_charWidthCharset1[224] = {
	0x06, 0x06, 0x04, 0x06, 0x07, 0x08, 0x08, 0x02, 0x04, 0x04, 0x06, 0x06, 0x06, 0x06, 0x06, 0x04,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x08, 0x04, 0x04, 0x04, 0x04, 0x07,
	0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x05, 0x07, 0x07, 0x03, 0x05, 0x07, 0x04, 0x08, 0x07, 0x07,
	0x07, 0x08, 0x07, 0x07, 0x04, 0x07, 0x07, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x07, 0x04, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08,
	0x08, 0x04, 0x08, 0x08, 0x07, 0x07, 0x06, 0x05, 0x07, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07
};

const uint8 TuckerEngine::_charWidthCharset2[58] = {
	0x13, 0x0F, 0x10, 0x10, 0x10, 0x0E, 0x11, 0x10, 0x0D, 0x0A, 0x11, 0x0D, 0x14, 0x13, 0x13, 0x11,
	0x13, 0x12, 0x10, 0x11, 0x13, 0x14, 0x14, 0x10, 0x13, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x13, 0x0F, 0x10, 0x10, 0x10, 0x0E, 0x11, 0x10, 0x0D, 0x0A, 0x11, 0x0D, 0x14, 0x13, 0x13, 0x11,
	0x13, 0x12, 0x10, 0x11, 0x13, 0x14, 0x14, 0x10, 0x13, 0x10,
};

const SoundSequenceData AnimationSequencePlayer::_soundSeqData[] = {
	{   16,  3, 1, 100 }, {   18,  5, 1, 100 }, {   26,  9, 5, 100 }, {   74,  9, 0, 100 },
	{  118, 10, 0, 100 }, {  139,  9, 0, 100 }, {  140, 10, 2, 100 }, {  144,  5, 2, 100 },
	{  146,  3, 2, 100 }, {  147,  9, 2, 100 }, {  148,  2, 1, 100 }, {  156,  0, 1,  50 },
	{  158,  4, 5, 100 }, {  188,  4, 5, 100 }, {  212,  2, 2, 100 }, {  223,  1, 0, 100 },
	{  230,  1, 2, 100 }, {  234,  1, 0, 100 }, {  251,  1, 0, 100 }, {  265, 11, 6,  80 },
	{  293,  5, 1,  80 }, {  295, 12, 6,  80 }, {  343, 13, 6, 110 }, {  373, 14, 6,  80 },
	{  429,  5, 2, 100 }, {  431, 15, 6,  80 }, {  456,  2, 1, 100 }, {  457,  1, 5, 100 },
	{  463,  0, 5, 100 }, {  482,  2, 2, 100 }, {  483,  8, 1, 100 }, {  494,  8, 5, 100 },
	{  507,  7, 5, 100 }, {  511,  7, 5, 100 }, {  522, 13, 1, 100 }, {  547,  6, 5, 100 },
	{  548, 11, 0, 100 }, {  556, 12, 0, 100 }, {  557,  2, 5, 100 }, {  599,  7, 1, 100 },
	{  639,  7, 2, 100 }, {  641, 12, 0, 100 }, {  642,  7, 5, 100 }, {  718, 14, 2, 100 },
	{  719,  1, 2, 100 }, {  720,  2, 2, 100 }, {  721,  3, 2, 100 }, {  722,  4, 2, 100 },
	{  723,  5, 2, 100 }, {  724,  6, 2, 100 }, {  725,  7, 2, 100 }, {  726,  8, 2, 100 },
	{  727,  9, 2, 100 }, {  728, 10, 2, 100 }, {  729, 11, 2, 100 }, {  730, 12, 2, 100 },
	{  731,  0, 2, 100 }, {  732, 13, 2, 100 },
	// count 2,58
	// offset 3,58
	{    2,  0, 3, 100 }, {    3,  2, 0, 100 }, {    3,  5, 1,  20 }, {    6, 25, 6,  80 },
	{   22,  1, 5, 100 }, {   37, 29, 6,  80 }, {   59, 30, 6,  80 }, {   69, 26, 6,  80 },
	{  124, 27, 6,  80 }, {  163, 28, 6,  80 }, {  197, 31, 6,  80 }, {  224, 32, 6,  80 },
	{  306, 33, 6,  80 }, {  350, 34, 6,  80 }, {  388,  8, 0, 100 }, {  389,  6, 1, 100 },
	{  390,  9, 1, 100 }, {  395, 10, 0, 100 }, {  398,  3, 0, 100 }, {  408,  3, 0, 100 },
	{  413,  4, 0, 100 }, {  418,  3, 0, 100 }, {  426,  6, 2, 100 }, {  427,  9, 2, 100 },
	{  430, 35, 6,  80 }, {  472, 36, 6,  80 }, {  514,  9, 1, 100 }, {  515,  7, 0, 100 },
	{  516,  6, 1, 100 }, {  517,  4, 0, 100 }, {  520, 11, 0, 100 }, {  523,  3, 0, 100 },
	{  531,  7, 2, 100 }, {  532,  9, 2, 100 }, {  533,  6, 2, 100 }, {  534,  3, 0, 100 },
	{  535, 37, 6,  80 }, {  553, 38, 6,  80 }, {  569, 39, 6,  80 }, {  588, 40, 6,  80 },
	{  662, 41, 6,  80 }, {  777, 42, 6,  80 }, {  978,  1, 0, 100 }, { 1008, 12, 0, 100 },
	{ 1011,  4, 5, 100 }, { 1022, 52, 6,  80 }, { 1039,  3, 5, 100 }, { 1045,  3, 5, 100 },
	{ 1051,  3, 5, 100 }, { 1057,  3, 5, 100 }, { 1063,  3, 5, 100 }, { 1066,  3, 5,  40 },
	{ 1074, 43, 6,  80 }, { 1256, 44, 6,  80 }, { 1322, 45, 6,  80 }, { 1373, 46, 6,  80 },
	{ 1419, 47, 6,  80 }, { 1792, 13, 0, 100 }, { 1793,  0, 5, 100 }, { 1895, 51, 6,  80 },
	// count 3,60
	// offset 4,118
	{    2,  0, 3, 100 }, {    4,  3, 1,  40 }, {    5,  7, 1,  20 }, {    7,  3, 5, 100 },
	{   62,  3, 2, 100 }, {   63, 48, 6,  80 }, {  198,  7, 2, 100 }, {  202,  5, 1, 100 },
	{  203, 16, 6,  80 }, {  211,  1, 5, 100 }, {  216,  0, 1,  40 }, {  217,  5, 2, 100 },
	{  218,  6, 1, 100 }, {  224,  2, 5, 100 }, {  229, 17, 6,  80 }, {  273,  6, 2, 100 },
	{  274,  4, 1, 100 }, {  275,  4, 5, 100 }, {  335,  1, 1, 100 }, {  336, 13, 0, 100 },
	{  339, 11, 0, 100 }, {  358,  0, 5, 100 }, {  364, 24, 4, 100 }, {  397,  7, 5, 100 },
	{  399,  6, 5, 100 }, {  433,  5, 5, 100 }, {  451, 13, 0, 100 }, {  459,  1, 2, 100 },
	{  508,  4, 2, 100 }, {  509,  1, 1, 100 }, {  521, 11, 0, 100 }, {  530,  8, 0, 100 },
	{  537,  9, 0, 100 }, {  557,  8, 5, 100 }, {  578,  5, 5, 100 }, {  602, 13, 0, 100 },
	{  609,  1, 2, 100 }, {  626, 12, 0, 100 }, {  628, 10, 0, 100 }, {  670,  6, 5, 100 },
	{  690, 10, 0, 100 }, {  692,  3, 5, 100 }, {  705, 11, 0, 100 }, {  710,  2, 5, 100 },
	{  714,  9, 0, 100 }, {  720,  9, 2, 100 }, {  721,  7, 5, 100 }, {  725,  0, 5, 100 },
	// count 4,48
	// offset 5,166
	{    3,  8, 0, 100 }, {   43, 11, 0, 100 }, {   68,  5, 1, 100 }, {  100,  5, 2, 100 },
	{  104,  6, 0, 100 }, {  139,  7, 0, 100 }, {  192,  3, 0, 100 }, {  199,  4, 0, 100 },
	{  207,  4, 0, 100 }, {  208,  2, 0, 100 }, {  254,  0, 0, 100 }, {  260,  0, 0, 100 },
	{  266,  0, 0, 100 }, {  272,  0, 0, 100 }, {  276,  0, 0, 100 }, {  282,  1, 0, 100 },
	{  303, 10, 0, 100 }, {  319, 12, 0, 100 }, {  355,  9, 0, 100 },
	// count 5,19
	// offset 6,185
	{    3,  0, 1, 100 }, {   34,  0, 2, 100 }, {   57,  0, 5, 100 }, {   77,  0, 5, 100 },
	{   84,  9, 0, 100 }, {  100,  9, 0, 100 }, {  140,  3, 5, 100 }, {  157,  2, 5, 100 },
	{  158,  1, 0, 100 }, {  204,  1, 5, 100 }, {  220,  2, 1, 100 }, {  235,  7, 0, 100 },
	{  236,  2, 2, 100 }, {  248, 12, 0, 100 }, {  251, 13, 0, 100 }, {  265, 11, 0, 100 },
	{  276, 11, 0, 100 }, {  310,  5, 0, 100 }, {  335,  6, 0, 100 }, {  370,  4, 0, 100 },
	{  373,  8, 0, 100 }, {  378,  8, 0, 100 }, {  434, 10, 0, 100 }, {  453,  3, 0, 100 },
	{  484,  3, 2, 100 },
	// count 6,25
	// offset 7,210
	{    3,  4, 5, 100 }, {   10,  5, 5, 100 }, {   22,  6, 5, 100 }, {   32,  7, 5, 100 },
	{   39,  6, 5, 100 }, {   48,  7, 5, 100 }, {   57,  6, 5, 100 }, {   65,  3, 0,  50 },
	{  102,  4, 0,  50 }, {  151,  2, 5,  50 }, {  205,  3, 5, 100 }, {  218,  0, 5, 100 },
	{  227,  0, 5, 100 }, {  236,  0, 5, 100 }, {  245,  0, 5, 100 }, {  254,  0, 5, 100 },
	{  263,  0, 5, 100 }, {  272,  0, 5, 100 }, {  291,  1, 1,  40 }, {  309,  8, 5, 100 },
	{  360,  8, 5, 100 }, {  384,  2, 1,  60 }, {  398, 10, 0, 100 }, {  440,  2, 2, 100 },
	{  441,  1, 2, 100 }, {  452, 11, 0, 100 }, {  466,  1, 5, 100 }, {  470,  8, 0, 100 },
	{  503,  6, 0, 100 }, {  519,  7, 0, 100 }, {  536,  8, 0, 100 }, {  538,  4, 5, 100 },
	{  549,  5, 5, 100 }, {  558,  4, 5, 100 }, {  568,  5, 5, 100 }, {  578,  4, 5, 100 },
	{  586,  5, 5, 100 }, {  593,  4, 5, 100 }, {  600,  5, 5, 100 }, {  606,  4, 5, 100 },
	{  613,  5, 5, 100 }, {  620,  9, 0, 100 }, {  630, 12, 0, 100 },
	// count 7,43
	// offset 8,253
	{   17,  2, 0, 100 }, {   57,  8, 0, 100 }, {   62, 10, 0, 100 }, {   67,  9, 0, 100 },
	{   74,  3, 1, 100 }, {   87,  3, 2, 100 }, {   85,  0, 0, 100 }, {  138,  5, 0, 100 },
	{  196,  7, 0, 100 }, {  222,  6, 0, 100 }, {  228,  1, 0, 100 },
	// count 8,11
	// offset 9,264
	{   20,  4, 0, 100 }, {  132,  0, 0, 100 }, {  145,  0, 2, 100 }, {  171,  1, 0, 100 },
	{  315,  2, 0,  50 }, {  334,  3, 0, 100 }, {  602,  5, 0,  50 }, {  631,  5, 0,  50 },
	{  660,  5, 0,  50 }, {  767,  5, 0,  50 }, {  795,  5, 0,  50 }, {  824,  5, 0,  50 },
	// count 9,12
	// offset 10,276
	{    4,  8, 1, 100 }, {    5,  6, 0, 100 }, {   47,  8, 2, 100 }, {  125,  8, 1, 100 },
	{  126,  6, 0, 100 }, {  156,  8, 2, 100 }, {  168,  2, 0, 100 }, {  174,  2, 0, 100 },
	{  180,  2, 0, 100 }, {  186,  2, 0, 100 }, {  192,  2, 0, 100 }, {  267,  2, 0, 100 },
	{  273,  2, 0, 100 }, {  279,  2, 0, 100 }, {  285,  2, 0, 100 }, {  291,  2, 0, 100 },
	{  297,  2, 0, 100 }, {  302,  0, 0, 100 }, {  417,  1, 0, 100 }, {  472,  5, 0, 100 },
	{  600,  5, 0, 100 }, {  623,  3, 0, 100 }, {  643,  9, 0, 100 }, {  781,  4, 0,  80 },
	// count 10,24
	// offset 11,300
	{    2,  0, 3, 100 }, {    3,  0, 1, 100 }, {   33,  0, 2, 100 }, {   33,  1, 0, 100 },
	{   53,  2, 1, 100 }, {   78,  2, 2, 100 }, {   80,  0, 4, 100 }
	// count 11,7
};

const char *AnimationSequencePlayer::_musicFileNamesTable[] = {
	"demomenu.raw",
	"introdub.raw",
	"introdua.raw",
	"demorola.raw",
	"democha1.raw",
	"icrmusic.raw",
	"demorolc.raw",
	"demorolc.raw",
	"merilogo.raw",
	"democha2.raw",
	"demorolc.raw",
	"int1.raw",
	"int2.raw",
	"int3.raw",
	"int4.raw",
	"int5.raw",
	"int6.raw",
	"int7.raw",
	"int8.raw",
	"int9.raw",
	"int10.raw",
	"int11.raw",
	"int12.raw",
	"int13.raw",
	"int14.raw",
	"int15.raw",
	"int16.raw",
	"int17.raw",
	"int18.raw",
	"int19.raw",
	"int20.raw",
	"int21.raw",
	"int22.raw",
	"int23.raw",
	"int24.raw",
	"int25.raw",
	"int26.raw",
	"int27.raw",
	"int28.raw",
	"int29.raw",
	"int30.raw",
	"int31.raw",
	"int32.raw",
	"int33.raw",
	"int34.raw",
	"int35.raw",
	"int36.raw",
	"int37.raw",
	"int38.raw",
	"int39.raw",
	"int40.raw",
	"int41.raw",
	"int42.raw"
};

const char *AnimationSequencePlayer::_audioFileNamesTable[] = {
	"introdua.wav",
	"fx35.wav",
	"fx47.wav",
	"fx50.wav",
	"fx51.wav",
	"fx52.wav",
	"fx54.wav",
	"fx58.wav",
	"fx59.wav",
	"fx53.wav",
	"fx55.wav",
	"fx68.wav",
	"fx57.wav",
	"fx36.wav",
	// offset 2,14
	"fx33.wav",
	"fx34.wav",
	"fx38.wav",
	"fx39.wav",
	"fx48.wav",
	"fx49.wav",
	"fx69.wav",
	"fx32.wav",
	"fx56.wav",
	"fx117.wav",
	// offset 3,24
	"introdua.wav",
	"fx37.wav",
	"fx49.wav",
	"fx72.wav",
	"fx73.wav",
	"fx74.wav",
	"fx75.wav",
	"fx76.wav",
	"fx77.wav",
	"fx78.wav",
	"fx79.wav",
	"fx80.wav",
	"fx81.wav",
	"fx83.wav",
	// offset 4,38
	"fx70.wav",
	"fx33.wav",
	"rdfx12.wav",
	"fx86.wav",
	"fx116.wav",
	// offset 5,43
	"introdua.wav",
	"fx36.wav",
	"fx46.wav",
	"fx52.wav",
	"fx59.wav",
	"fx66.wav",
	"fx67.wav",
	"fx74.wav",
	"rdfx26.wav",
	"fx42.wav",
	"fx43.wav",
	"fx44.wav",
	"fx45.wav",
	"fx61.wav",
	// offset 6,57
	"fx40.wav",
	"fx60.wav",
	"fx62.wav",
	"fx71.wav",
	"fx63.wav",
	"fx64.wav",
	"fx68.wav",
	"fx69.wav",
	"rdfx12.wav",
	// offset 7,66
	"rdfx1.wav",
	"rdfx2.wav",
	"rdfx4.wav",
	"rdfx5.wav",
	"rdfx6.wav",
	"rdfx7.wav",
	"rdfx10.wav",
	"rdfx11.wav",
	"rdfx12.wav",
	"rdfx19.wav",
	"rdfx29.wav",
	"rdfx32.wav",
	"rdfx35.wav",
	// offset 8,79
	"rdfx8.wav",
	"rdfx9.wav",
	"rdfx13.wav",
	"rdfx14.wav",
	"rdfx33.wav",
	"rdfx16.wav",
	"rdfx18.wav",
	"rdfx20.wav",
	"rdfx38.wav",
	"fx39.wav",
	"rdfx28.wav",
	"rdfx25.wav",
	"rdfx26.wav",
	"rdfx27.wav",
	// offset 9,93
	"rdfx31.wav",
	"rdfx21.wav",
	"rdfx15.wav",
	"fx34.wav",
	// offset 10,97
	"fx96.wav",
	"fx74.wav",
	"fx75.wav",
	"fx97.wav",
	"fx98.wav",
	"fx106.wav",
	"fx107.wav",
	"fx108.wav",
	"fx109.wav",
	"fx110.wav",
	"fx93.wav",
	"fx83.wav",
	"fx111.wav",
	// offset 11,110
	"fx86.wav",
	"fx101.wav",
	"fx112.wav",
	"fx104.wav",
	"fx102.wav",
	"fx103.wav",
	"fx91.wav",
	"fx92.wav",
	"fx105.wav",
	// offset 12,119
	"rdfx3.wav",
	"rdfx9.wav",
	"rdfx17.wav",
	"rdfx22.wav",
	"rdfx23.wav",
	"rdfx24.wav",
	"rdfx30.wav",
	"rdfx36.wav",
	"rdfx1.wav",
	"rdfx16.wav",
	"rdfx37.wav",
	// offset 13,130
	"fx82.wav",
	"fx80.wav",
	"fx89.wav",
	"fx90.wav",
	"fx83.wav",
	"fx84.wav",
	// offset 14,136
	"rdfx2.wav",
	"fx85.wav",
	"fx86.wav",
	"fx87.wav",
	"fx88.wav",
	"fx99.wav",
	"fx98.wav",
	"fx97.wav",
	"fx96.wav",
	"fx100.wav",
	// offset 15,146
	"rdfx22.wav",
	"fx113.wav",
	"fx114.wav",
	"fx115.wav"
};

} // namespace Tucker
