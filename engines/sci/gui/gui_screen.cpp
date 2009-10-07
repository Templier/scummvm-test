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

#include "common/timer.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_screen.h"

namespace Sci {

SciGuiScreen::SciGuiScreen(int16 width, int16 height, int16 scaleFactor) : 
	_width(width), _height(height) {

	int i;
	uint16 base = 0;

	_pixels = _width * _height;

	// if you want to do scaling, adjust putPixel() accordingly
	_displayWidth = _width * scaleFactor;
	_displayHeight = _height * scaleFactor;
	_displayPixels = _displayWidth * _displayHeight;

	_visualScreen = initScreen(_pixels);
	_priorityScreen = initScreen(_pixels);
	_controlScreen = initScreen(_pixels);
	_displayScreen = initScreen(_displayPixels);

	// Sets display screen to be actually displayed
	_activeScreen = _displayScreen;

	for (i = 0; i < _height; i++) {
		_baseTable[i] = base; _baseDisplayTable[i] = base;
		base += _width;
	}

	_picNotValid = false;
}

SciGuiScreen::~SciGuiScreen() {
	free(_visualScreen);
	free(_priorityScreen);
	free(_controlScreen);
	free(_displayScreen);
}

byte *SciGuiScreen::initScreen(uint16 pixelCount) {
	byte *screen = (byte *)malloc(pixelCount);
	memset(screen, 0, pixelCount);
	return screen;
}

void SciGuiScreen::copyToScreen() {
	g_system->copyRectToScreen(_activeScreen, _displayWidth, 0, 0, _displayWidth, _displayHeight);
}

byte SciGuiScreen::getDrawingMask(byte color, byte prio, byte control) {
	byte flag = 0;
	if (color != 255)
		flag |= SCI_SCREEN_MASK_VISUAL;
	if (prio != 255)
		flag |= SCI_SCREEN_MASK_PRIORITY;
	if (control != 255)
		flag |= SCI_SCREEN_MASK_CONTROL;
	return flag;
}

void SciGuiScreen::putPixel(int x, int y, byte drawMask, byte color, byte priority, byte control) {
	int offset = _baseTable[y] + x;

	if (drawMask & SCI_SCREEN_MASK_VISUAL) {
		*(_visualScreen + offset) = color;
		_displayScreen[_baseDisplayTable[y] + x] = color;
	}
	if (drawMask & SCI_SCREEN_MASK_PRIORITY)
		*(_priorityScreen + offset) = priority;
	if (drawMask & SCI_SCREEN_MASK_CONTROL)
		*(_controlScreen + offset) = control;
}

byte SciGuiScreen::getVisual(int x, int y) {
	return _visualScreen[_baseTable[y] + x];
}

byte SciGuiScreen::getPriority(int x, int y) {
	return _priorityScreen[_baseTable[y] + x];
}

byte SciGuiScreen::getControl(int x, int y) {
	return _controlScreen[_baseTable[y] + x];
}

byte SciGuiScreen::isFillMatch(int16 x, int16 y, byte flag, byte t_color, byte t_pri, byte t_con) {
	int offset = _baseTable[y] + x;
	byte match = 0;

	if (flag & SCI_SCREEN_MASK_VISUAL && *(_visualScreen + offset) == t_color)
		match |= SCI_SCREEN_MASK_VISUAL;
	if (flag & SCI_SCREEN_MASK_PRIORITY && *(_priorityScreen + offset) == t_pri)
		match |= SCI_SCREEN_MASK_PRIORITY;
	if (flag & SCI_SCREEN_MASK_CONTROL && *(_controlScreen + offset) == t_con)
		match |= SCI_SCREEN_MASK_CONTROL;
	return match;
}

int SciGuiScreen::getBitsDataSize(Common::Rect rect, byte mask) {
	int byteCount = sizeof(rect) + sizeof(mask);
	int pixels = rect.width() * rect.height();
	if (mask & SCI_SCREEN_MASK_VISUAL) {
		byteCount += pixels; // _visualScreen
		byteCount += pixels; // _displayScreen
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		byteCount += pixels; // _priorityScreen
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		byteCount += pixels; // _controlScreen
	}

	return byteCount;
}

void SciGuiScreen::saveBits(Common::Rect rect, byte mask, byte *memoryPtr) {
	memcpy(memoryPtr, (void *)&rect, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy(memoryPtr, (void *)&mask, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		saveBitsScreen(rect, _visualScreen, memoryPtr);
		saveBitsScreen(rect, _displayScreen, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		saveBitsScreen(rect, _priorityScreen, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		saveBitsScreen(rect, _controlScreen, memoryPtr);
	}
}

void SciGuiScreen::saveBitsScreen(Common::Rect rect, byte *screen, byte *&memoryPtr) {
	int width = rect.width();
	int y;

	screen += (rect.top * _width) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy(memoryPtr, (void*)screen, width); memoryPtr += width;
		screen += _width;
	}
}

void SciGuiScreen::restoreBits(byte *memoryPtr) {
	Common::Rect rect;
	byte mask;

	memcpy((void *)&rect, memoryPtr, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy((void *)&mask, memoryPtr, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		restoreBitsScreen(rect, memoryPtr, _visualScreen);
		restoreBitsScreen(rect, memoryPtr, _displayScreen);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		restoreBitsScreen(rect, memoryPtr, _priorityScreen);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		restoreBitsScreen(rect, memoryPtr, _controlScreen);
	}
}

void SciGuiScreen::restoreBitsScreen(Common::Rect rect, byte *&memoryPtr, byte *screen) {
	int width = rect.width();
	int y;

	screen += (rect.top * _width) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy((void*) screen, memoryPtr, width); memoryPtr += width;
		screen += _width;
	}
}

void SciGuiScreen::setPalette(GuiPalette*pal) {
	// just copy palette to system
	byte bpal[4 * 256];
	// Get current palette, update it and put back
	g_system->grabPalette(bpal, 0, 256);
	for (int16 i = 0; i < 256; i++) {
		if (!pal->colors[i].used)
			continue;
		bpal[i * 4] = pal->colors[i].r * pal->intensity[i] / 100;
		bpal[i * 4 + 1] = pal->colors[i].g * pal->intensity[i] / 100;
		bpal[i * 4 + 2] = pal->colors[i].b * pal->intensity[i] / 100;
		bpal[i * 4 + 3] = 100;
	}
	g_system->setPalette(bpal, 0, 256);
}

// Currently not really done, its supposed to be possible to only dither _visualScreen
void SciGuiScreen::dither() {
	int y, x;
	byte color;
	byte *screenPtr = _visualScreen;
	byte *displayPtr = _displayScreen;

	for (y = 0; y < _height; y++) {
		for (x = 0; x < _width; x++) {
			color = *screenPtr;
			if (color & 0xF0) {
				color ^= color << 4;
				color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
				*screenPtr = color; *displayPtr = color;
			}
			screenPtr++; displayPtr++;
		}
	}
}

void SciGuiScreen::debugShowMap(int mapNo) {
	switch (mapNo) {
	case 0:
		_activeScreen = _visualScreen;
		break;
	case 1:
		_activeScreen = _priorityScreen;
		break;
	case 2:
		_activeScreen = _controlScreen;
		break;
	case 3:
		_activeScreen = _displayScreen;
		break;
	}
}

} // End of namespace Sci
