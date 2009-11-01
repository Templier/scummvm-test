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

#ifndef DRACI_WALKING_H
#define DRACI_WALKING_H

#include "common/array.h"
#include "common/rect.h"

namespace Draci {

class Sprite;

struct PathVertex {
	PathVertex() {}
	PathVertex(int xx, int yy) : x(xx), y(yy) {}
	int x, y;
};

class WalkingMap {
public:
	WalkingMap() : _realWidth(0), _realHeight(0), _deltaX(1), _deltaY(1),
		_mapWidth(0), _mapHeight(0), _byteWidth(0), _data(NULL) { }

	void load(const byte *data, uint length);

	bool getPixel(int x, int y) const;
	bool isWalkable(int x, int y) const;

	Sprite *newOverlayFromMap(byte colour) const;
	Common::Point findNearestWalkable(int x, int y, Common::Rect searchRect) const;

	typedef Common::Array<PathVertex> Path;
	bool findShortestPath(int x1, int y1, int x2, int y2, Path *path) const;
	void obliquePath(const Path& path, Path *obliquedPath) const;
	Sprite *newOverlayFromPath(const Path &path, byte colour) const;

private:
	int _realWidth, _realHeight;
	int _deltaX, _deltaY;
	int _mapWidth, _mapHeight;
	int _byteWidth;

	// We don't own the pointer.  It points to the BArchive cache for this room.
	const byte *_data;

	// 4 possible directions to walk from a pixel.
	static int kDirections[][2];

	void drawOverlayRectangle(int x, int y, byte colour, byte *buf) const;
};

/*
 * Enumerates the directions the dragon can look into when arrived.
 */
enum SightDirection {
	kDirectionLast, kDirectionMouse, kDirectionUnknown,
	kDirectionRight, kDirectionLeft, kDirectionIntelligent
};

/**
  * Enumerates the animations for the dragon's movement.
  */
enum Movement {
	kMoveUndefined = -1,
	kMoveDown, kMoveUp, kMoveRight, kMoveLeft,
	kMoveRightDown, kMoveRightUp, kMoveLeftDown, kMoveLeftUp,
	kMoveDownRight, kMoveUpRight, kMoveDownLeft, kMoveUpLeft,
	kMoveLeftRight, kMoveRightLeft, kMoveUpStopLeft, kMoveUpStopRight,
	kSpeakRight, kSpeakLeft, kStopRight, kStopLeft
};

} // End of namespace Draci

#endif // DRACI_WALKING_H
