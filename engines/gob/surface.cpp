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

#include "gob/surface.h"

#include "common/system.h"
#include "common/util.h"
#include "common/frac.h"

#include "graphics/primitives.h"

namespace Gob {

static void plotPixel(int x, int y, int color, void *data) {
	Surface *dest = (Surface *)data;

	dest->putPixel(x, y, color);
}


Pixel::Pixel(byte *vidMem, uint8 bpp) : _vidMem(vidMem), _bpp(bpp) {
	assert((_bpp == 1) || (_bpp == 2));
}

Pixel &Pixel::operator++() {
	_vidMem += _bpp;
	return *this;
}

Pixel Pixel::operator++(int x) {
	Pixel p = *this;
	++(*this);
	return p;
}

Pixel &Pixel::operator--() {
	_vidMem -= _bpp;
	return *this;
}

Pixel Pixel::operator--(int x) {
	Pixel p = *this;
	--(*this);
	return p;
}

Pixel &Pixel::operator+=(int x) {
	_vidMem += x * _bpp;
	return *this;
}

Pixel &Pixel::operator-=(int x) {
	_vidMem -= x * _bpp;
	return *this;
}

uint32 Pixel::get() const {
	if (_bpp == 1)
		return *((byte *) _vidMem);
	if (_bpp == 2)
		return *((uint16 *) _vidMem);

	return 0;
}

void Pixel::set(uint32 p) {
	if (_bpp == 1)
		*((byte *) _vidMem) = (byte) p;
	if (_bpp == 2)
		*((uint16 *) _vidMem) = (uint16) p;
}


ConstPixel::ConstPixel(const byte *vidMem, uint8 bpp) : _vidMem(vidMem), _bpp(bpp) {
	assert((_bpp == 1) || (_bpp == 2));
}

ConstPixel &ConstPixel::operator++() {
	_vidMem += _bpp;
	return *this;
}

ConstPixel ConstPixel::operator++(int x) {
	ConstPixel p = *this;
	++(*this);
	return p;
}

ConstPixel &ConstPixel::operator--() {
	_vidMem -= _bpp;
	return *this;
}

ConstPixel ConstPixel::operator--(int x) {
	ConstPixel p = *this;
	--(*this);
	return p;
}

ConstPixel &ConstPixel::operator+=(int x) {
	_vidMem += x * _bpp;
	return *this;
}

ConstPixel &ConstPixel::operator-=(int x) {
	_vidMem -= x * _bpp;
	return *this;
}

uint32 ConstPixel::get() const {
	if (_bpp == 1)
		return *((const byte *) _vidMem);
	if (_bpp == 2)
		return *((const uint16 *) _vidMem);

	return 0;
}


Surface::Surface(uint16 width, uint16 height, uint8 bpp, byte *vidMem) :
	_width(width), _height(height), _bpp(bpp), _vidMem(vidMem) {

	assert((_width > 0) && (_height > 0));
	assert((_bpp == 1) || (_bpp == 2));

	if (!_vidMem) {
		_vidMem    = new byte[_bpp * _width * _height];
		_ownVidMem = true;

		memset(_vidMem, 0, _bpp * _width * _height);
	} else
		_ownVidMem = false;
}

Surface::~Surface() {
	if (_ownVidMem)
		delete[] _vidMem;
}

uint16 Surface::getWidth() const {
	return _width;
}

uint16 Surface::getHeight() const {
	return _height;
}

uint8 Surface::getBPP() const {
	return _bpp;
}

void Surface::resize(uint16 width, uint16 height) {
	assert((width > 0) && (height > 0));

	if (_ownVidMem)
		delete[] _vidMem;

	_width  = width;
	_height = height;

	_vidMem    = new byte[_bpp * _width * _height];
	_ownVidMem = true;

	memset(_vidMem, 0, _bpp * _width * _height);
}

void Surface::setBPP(uint8 bpp) {
	if (_bpp == bpp)
		return;

	if (_ownVidMem) {
		delete[] _vidMem;

		_vidMem = new byte[bpp * _width * _height];
	} else
		_width = (_width * _bpp) / bpp;

	_bpp = bpp;

	memset(_vidMem, 0, _bpp * _width * _height);
}

byte *Surface::getData(uint16 x, uint16 y) {
	return _vidMem + (y * _width * _bpp) + (x * _bpp);
}

const byte *Surface::getData(uint16 x, uint16 y) const {
	return _vidMem + (y * _width * _bpp) + (x * _bpp);
}

Pixel Surface::get(uint16 x, uint16 y) {
	byte *vidMem = getData(x, y);

	return Pixel(vidMem, _bpp);
}

ConstPixel Surface::get(uint16 x, uint16 y) const {
	const byte *vidMem = getData(x, y);

	return ConstPixel(vidMem, _bpp);
}

bool Surface::clipBlitRect(int16 &left, int16 &top, int16 &right, int16 &bottom, int16 &x, int16 &y,
		uint16 dWidth, uint16 dHeight, uint16 sWidth, uint16 sHeight) {

	if ((x >= dWidth) || (y >= dHeight))
		// Nothing to do
		return false;

	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= sWidth) || (top >= sHeight) || (right < 0) || (bottom < 0))
		// Nothing to do
		return false;

	// Adjust from coordinates
	if (left < 0) {
		x   -= left;
		left = 0;
	}
	if (top < 0) {
		y  -= top;
		top = 0;
	}

	// Adjust to coordinates
	if (x < 0) {
		left -= x;
		x     = 0;
	}
	if (y < 0) {
		top -= y;
		y    = 0;
	}

	// Limit by source and destination dimensions
	right  = MIN<int32>(right , MIN<int32>(sWidth , dWidth  - x + left) - 1);
	bottom = MIN<int32>(bottom, MIN<int32>(sHeight, dHeight - y + top ) - 1);

	if ((right < left) || (bottom < top))
		// Nothing to do
		return false;

	// Clip to sane values
	right  = MAX<int16>(right , 0);
	bottom = MAX<int16>(bottom, 0);

	return true;
}

void Surface::blit(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
		int16 x, int16 y, int32 transp) {

	// Color depths have to fit
	assert(_bpp == from._bpp);

	// Clip
	if (!clipBlitRect(left, top, right, bottom, x, y, _width, _height, from._width, from._height))
		return;

	// Area to actually copy
	uint16 width  = right  - left + 1;
	uint16 height = bottom - top  + 1;

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	if ((left == 0) && (_width == from._width) && (_width == width) && (transp == -1)) {
		// If these conditions are met, we can directly use memcpy

		// Pointers to the blit destination and source start points
		      byte *dst =      getData(x   , y);
		const byte *src = from.getData(left, top);

		memcpy(dst, src, width * height * _bpp);
		return;
	}

	if (transp == -1) {
		// We don't have to look for transparency => we can use memcpy line-wise

		// Pointers to the blit destination and source start points
		      byte *dst =      getData(x   , y);
		const byte *src = from.getData(left, top);

		while (height-- > 0) {
			memcpy(dst, src, width * _bpp);

			dst +=      _width *      _bpp;
			src += from._width * from._bpp;
		}

		return;
	}

	// Otherwise, we have to copy by pixel

	// Pointers to the blit destination and source start points
	     Pixel dst =      get(x   , y);
	ConstPixel src = from.get(left, top);

	while (height-- > 0) {
		     Pixel dstRow = dst;
		ConstPixel srcRow = src;

		for (uint16 i = 0; i < width; i++, dstRow++, srcRow++)
			if (srcRow.get() != ((uint32) transp))
				dstRow.set(srcRow.get());

		dst +=      _width;
		src += from._width;
	}
}

void Surface::blit(const Surface &from, int16 x, int16 y, int32 transp) {
	blit(from, 0, 0, from._width - 1, from._height - 1, x, y, transp);
}

void Surface::blit(const Surface &from, int32 transp) {
	blit(from, 0, 0, from._width - 1, from._height - 1, 0, 0, transp);
}

void Surface::blitScaled(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
		int16 x, int16 y, Common::Rational scale, int32 transp) {

	if (scale == 1) {
		// Yeah, "scaled"

		blit(from, left, top, right, bottom, x, y, transp);
		return;
	}

	// Color depths have to fit
	assert(_bpp == from._bpp);

	uint16 dWidth  = (uint16) floor((_width  / scale).toDouble());
	uint16 dHeight = (uint16) floor((_height / scale).toDouble());

	// Clip
	if (!clipBlitRect(left, top, right, bottom, x, y, dWidth, dHeight, from._width, from._height))
		return;

	// Area to actually copy
	uint16 width  = right  - left + 1;
	uint16 height = bottom - top  + 1;

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	width  = MIN<int32>((int32) floor((width  * scale).toDouble()), _width);
	height = MIN<int32>((int32) floor((height * scale).toDouble()), _height);

	// Pointers to the blit destination and source start points
	      byte *dst =      getData(x   , y);
	const byte *src = from.getData(left, top);

	frac_t step = scale.getInverse().toFrac();

	frac_t posW = 0, posH = 0;
	while (height-- > 0) {
		      byte *dstRow = dst;
		const byte *srcRow = src;

		posW = 0;

		for (uint16 i = 0; i < width; i++, dstRow += _bpp) {
			memcpy(dstRow, srcRow, _bpp);

			posW += step;
			while (posW >= ((frac_t) FRAC_ONE)) {
				srcRow += from._bpp;
				posW   -= FRAC_ONE;
			}
		}

		posH += step;
		while (posH >= ((frac_t) FRAC_ONE)) {
			src  += from._width * from._bpp;
			posH -= FRAC_ONE;
		}

		dst += _width * _bpp;
	}

}

void Surface::blitScaled(const Surface &from, int16 x, int16 y, Common::Rational scale, int32 transp) {
	blitScaled(from, 0, 0, from._width - 1, from._height - 1, x, y, scale, transp);
}

void Surface::blitScaled(const Surface &from, Common::Rational scale, int32 transp) {
	blitScaled(from, 0, 0, from._width - 1, from._height - 1, 0, 0, scale, transp);
}

void Surface::fillRect(uint16 left, uint16 top, uint16 right, uint16 bottom, uint32 color) {
	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= _width) || (top >= _height))
		// Nothing to do
		return;

	// Area to actually fill
	uint16 width  = CLIP<int32>(right  - left + 1, 0, _width  - left);
	uint16 height = CLIP<int32>(bottom - top  + 1, 0, _height - top);

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	if ((left == 0) && (width == _width) && (_bpp == 1)) {
		// We can directly use memset

		byte *dst = getData(left, top);

		memset(dst, (byte) color, width * height);
		return;
	}

	if (_bpp == 1) {
		// We can use memset line-wise

		byte *dst = getData(left, top);

		while (height-- > 0) {
			memset(dst, (byte) color, width);
			dst += _width;
		}

		return;
	}

	assert(_bpp == 2);

	// Otherwise, we have to fill by pixel

	Pixel p = get(left, top);
	while (height-- > 0) {
		for (uint16 i = 0; i < width; i++, ++p)
			p.set(color);

		p += _width - width;
	}
}

void Surface::fill(uint32 color) {
	if (_bpp == 1) {
		// We can directly use memset

		memset(_vidMem, (byte) color, _width * _height);
		return;
	}

	fillRect(0, 0, _width - 1, _height - 1, color);
}

void Surface::clear() {
	fill(0);
}

void Surface::shadeRect(uint16 left, uint16 top, uint16 right, uint16 bottom,
		uint32 color, uint8 strength) {

	if (_bpp == 1) {
		// We can't properly shade in paletted mode, fill the rect instead
		fillRect(left, top, right, bottom, color);
		return;
	}

	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= _width) || (top >= _height))
		// Nothing to do
		return;

	// Area to actually shade
	uint16 width  = CLIP<int32>(right  - left + 1, 0, _width  - left);
	uint16 height = CLIP<int32>(bottom - top  + 1, 0, _height - top);

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();

	uint8 cR, cG, cB;
	pixelFormat.colorToRGB(color, cR, cG, cB);

	int shadeR = cR * (16 - strength);
	int shadeG = cG * (16 - strength);
	int shadeB = cB * (16 - strength);

	Pixel p = get(left, top);
	while (height-- > 0) {
		for (uint16 i = 0; i < width; i++, ++p) {
			uint8 r, g, b;

			pixelFormat.colorToRGB(p.get(), r, g, b);

			r = CLIP<int>((shadeR + strength * r) >> 4, 0, 255);
			g = CLIP<int>((shadeG + strength * g) >> 4, 0, 255);
			b = CLIP<int>((shadeB + strength * b) >> 4, 0, 255);

			p.set(pixelFormat.RGBToColor(r, g, b));
		}

		p += _width - width;
	}

}

void Surface::putPixel(uint16 x, uint16 y, uint32 color) {
	if ((x >= _width) || (y >= _height))
		return;

	get(x, y).set(color);
}

void Surface::drawLine(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint32 color) {
	Graphics::drawLine(x0, y0, x1, y1, color, &plotPixel, this);
}

/*
 * The original's version of the Bresenham Algorithm was a bit "unclean"
 * and produced strange edges at 45, 135, 225 and 315 degrees, so using the
 * version found in the Wikipedia article about the
 * "Bresenham's line algorithm" instead
 */
void Surface::drawCircle(uint16 x0, uint16 y0, uint16 radius, uint32 color, int16 pattern) {
	int16 f = 1 - radius;
	int16 ddFx = 0;
	int16 ddFy = -2 * radius;
	int16 x = 0;
	int16 y = radius;

	if (pattern == 0) {
		putPixel(x0, y0 + radius, color);
		putPixel(x0, y0 - radius, color);
		putPixel(x0 + radius, y0, color);
		putPixel(x0 - radius, y0, color);
	} else
		warning("Surface::drawCircle - pattern %d", pattern);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddFy += 2;
			f += ddFy;
		}
		x++;
		ddFx += 2;
		f += ddFx + 1;

		switch (pattern) {
		case -1:
			fillRect(x0 - y, y0 + x, x0 + y, y0 + x, color);
			fillRect(x0 - x, y0 + y, x0 + x, y0 + y, color);
			fillRect(x0 - y, y0 - x, x0 + y, y0 - x, color);
			fillRect(x0 - x, y0 - y, x0 + x, y0 - y, color);
			break;
		case 0:
			putPixel(x0 + x, y0 + y, color);
			putPixel(x0 - x, y0 + y, color);
			putPixel(x0 + x, y0 - y, color);
			putPixel(x0 - x, y0 - y, color);
			putPixel(x0 + y, y0 + x, color);
			putPixel(x0 - y, y0 + x, color);
			putPixel(x0 + y, y0 - x, color);
			putPixel(x0 - y, y0 - x, color);
			break;
		default:
			fillRect(x0 + y - pattern, y0 + x - pattern, x0 + y, y0 + x, color);
			fillRect(x0 + x - pattern, y0 + y - pattern, x0 + x, y0 + y, color);
			fillRect(x0 - y, y0 + x - pattern, x0 - y + pattern, y0 + x, color);
			fillRect(x0 - x, y0 + y - pattern, x0 - x + pattern, y0 + y, color);
			fillRect(x0 + y - pattern, y0 - x, x0 + y, y0 - x + pattern, color);
			fillRect(x0 + x - pattern, y0 - y, x0 + x, y0 - y + pattern, color);
			fillRect(x0 - y, y0 - x, x0 - y + pattern, y0 - x + pattern, color);
			fillRect(x0 - x, y0 - y, x0 - x + pattern, y0 - y + pattern, color);
			break;
		}
	}
}

void Surface::blitToScreen(uint16 left, uint16 top, uint16 right, uint16 bottom, uint16 x, uint16 y) const {
	// Color depths have to fit
	assert(g_system->getScreenFormat().bytesPerPixel == _bpp);

	uint16 sWidth  = g_system->getWidth();
	uint16 sHeight = g_system->getHeight();

	if ((x >= sWidth) || (y >= sHeight))
		// Nothing to do
		return;

	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= _width) || (top >= _height))
		// Nothing to do
		return;

	// Area to actually copy
	uint16 width  = MAX<int32>(MIN<int32>(MIN<int32>(right  - left + 1, _width  - left), sWidth  - x), 0);
	uint16 height = MAX<int32>(MIN<int32>(MIN<int32>(bottom - top  + 1, _height - top ), sHeight - y), 0);

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	// Pointers to the blit destination and source start points
	const byte *src = getData(left, top);

	g_system->copyRectToScreen(src, _width * _bpp, x, y, width, height);
}

} // End of namespace Gob
