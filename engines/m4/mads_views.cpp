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

#include "m4/m4_views.h"
#include "m4/dialogs.h"
#include "m4/events.h"
#include "m4/font.h"
#include "m4/globals.h"
#include "m4/mads_menus.h"
#include "m4/m4.h"
#include "m4/staticres.h"

#include "common/algorithm.h"

namespace M4 {

static const int INV_ANIM_FRAME_SPEED = 2;
static const int INVENTORY_X = 160;
static const int INVENTORY_Y = 159;
static const int SCROLLER_DELAY = 200;

//--------------------------------------------------------------------------

MadsSpriteSlots::MadsSpriteSlots(MadsView &owner): _owner(owner) {
	for (int i = 0; i < SPRITE_SLOTS_SIZE; ++i) {
		MadsSpriteSlot rec;
		_entries.push_back(rec);
	}

	startIndex = 0;
}

void MadsSpriteSlots::clear() {
	_owner._textDisplay.clear();
	_sprites.clear();

	// Reset the sprite slots list back to a single entry for a full screen refresh
	startIndex = 1;
	_entries[0].spriteType = FULL_SCREEN_REFRESH;
	_entries[0].timerIndex = -1;
}

int MadsSpriteSlots::getIndex() {
	if (startIndex == SPRITE_SLOTS_SIZE)
		error("Run out of sprite slots");

	return startIndex++;
}

int MadsSpriteSlots::addSprites(const char *resName) {
	// Get the sprite set
	Common::SeekableReadStream *data = _vm->res()->get(resName);
	SpriteAsset *spriteSet = new SpriteAsset(_vm, data, data->size(), resName);
	spriteSet->translate(_madsVm->_palette);

	_sprites.push_back(SpriteList::value_type(spriteSet));
	_vm->res()->toss(resName);

	return _sprites.size() - 1;
}

/*
 * Deletes the sprite slot with the given timer entry
 */
void MadsSpriteSlots::deleteTimer(int timerIndex) {
	for (int idx = 0; idx < startIndex; ++idx) {
		if (_entries[idx].timerIndex == timerIndex)
			_entries[idx].spriteType = -1;
	}
}

class DepthEntry {
public:
	int depth;
	int index;

	DepthEntry(int depthAmt, int indexVal) { depth = depthAmt; index = indexVal; }
};

bool sortHelper(const DepthEntry &entry1, const DepthEntry &entry2) {
	return entry1.depth < entry2.depth;
}

typedef Common::List<DepthEntry> DepthList;

void MadsSpriteSlots::drawBackground() {
	// Draw all active sprites onto the background surface
	for (int i = 0; i < startIndex; ++i) {
		if (_entries[i].spriteType >= 0) {
			_owner._dirtyAreas[i].active = false;
		} else {
			_owner._dirtyAreas[i].textActive = true;
			_owner._dirtyAreas.setSpriteSlot(i, _entries[i]);

			if (_entries[i].spriteType == BACKGROUND_SPRITE) {
				SpriteAsset &spriteSet = getSprite(_entries[i].spriteListIndex);
				M4Sprite *frame = spriteSet.getFrame((_entries[i].frameNumber & 0x7fff) - 1);
				int xp = _entries[i].xp;
				int yp = _entries[i].yp;

				if (_entries[i].scale != -1) {
					// Adjust position based on frame size
					xp -= frame->width() / 2;
					yp -= frame->height() / 2;
				}

				if (_entries[i].depth <= 1) {
					// No depth, so simply copy the frame onto the background
					frame->copyTo(_owner._bgSurface, xp, yp);
				} else {
					// Depth was specified, so draw frame using scene's depth information
					frame->copyTo(_owner._bgSurface, xp, yp, _entries[i].depth, _owner._depthSurface, 100);
				}
			}
		}
	}

	// Flag any remaining dirty areas as inactive
	for (uint i = startIndex; i < DIRTY_AREAS_TEXT_DISPLAY_IDX; ++i)
		_owner._dirtyAreas[i].active = false;
}

void MadsSpriteSlots::drawForeground(View *view) {
	DepthList depthList;

	// Get a list of sprite object depths for active objects
	for (int i = 0; i < startIndex; ++i) {
		if (_entries[i].spriteType >= 0) {
			DepthEntry rec(_entries[i].depth, i);
			depthList.push_back(rec);
		}
	}

	// Sort the list in order of the depth
	Common::sort(depthList.begin(), depthList.end(), sortHelper);

	// Loop through each of the objects
	DepthList::iterator i;
	for (i = depthList.begin(); i != depthList.end(); ++i) {
		DepthEntry &de = *i;
		MadsSpriteSlot &slot = _entries[de.index];
		assert(slot.spriteListIndex < (int)_sprites.size());
		SpriteAsset &spriteSet = *_sprites[slot.spriteListIndex].get();

		if (slot.scale < 100) {
			// Minimalised drawing
			assert(slot.spriteListIndex < (int)_sprites.size());
			M4Sprite *spr = spriteSet.getFrame(slot.frameNumber - 1);
			spr->copyTo(view, slot.xp, slot.yp, slot.depth, _owner._depthSurface, slot.scale, 0);
		} else {
			int xp, yp;
			M4Sprite *spr = spriteSet.getFrame(slot.frameNumber - 1);

			if (slot.scale == -1) {
				xp = slot.xp - _owner._posAdjust.x;
				yp = slot.yp - _owner._posAdjust.y;
			} else {
				xp = slot.xp - (spr->width() / 2) - _owner._posAdjust.x;
				yp = slot.yp - spr->height() - _owner._posAdjust.y + 1;
			}

			if (slot.depth > 1) {
				// Draw the frame with depth processing
				spr->copyTo(view, xp, yp, slot.depth, _owner._depthSurface, 100, 0);
			} else {
				// No depth, so simply draw the image
				spr->copyTo(view, xp, yp, 0);
			}
		}
	}
}

void MadsSpriteSlots::setDirtyAreas() {
	for (int i = 0; i < startIndex; ++i) {
		if (_entries[i].spriteType >= 0) {
			_owner._dirtyAreas.setSpriteSlot(i, _entries[i]);

			_owner._dirtyAreas[i].textActive = (_entries[i].spriteType <= 0) ? 0 : 1;
			_entries[i].spriteType = 0;
		}
	}
}

/**
 * Removes any sprite slots that are no longer needed
 */
void MadsSpriteSlots::cleanUp() {
	// Delete any entries that aren't needed
	int idx = 0;
	while (idx < startIndex) {
		if (_entries[idx].spriteType < 0) {
			_entries.remove_at(idx);
			--startIndex;
		} else {
			++idx;
		}
	}

	// Original engine sprite slot list was a fixed array, so to keep the engine similiar, for
	// now I'm adding in new entries to make up the original fixed total again
	while (_entries.size() < SPRITE_SLOTS_SIZE) {
		MadsSpriteSlot rec;
		_entries.push_back(rec);
	}
}

//--------------------------------------------------------------------------

MadsTextDisplay::MadsTextDisplay(MadsView &owner): _owner(owner) {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i) {
		MadsTextDisplayEntry rec;
		rec.active = false;
		_entries.push_back(rec);
	}
}

void MadsTextDisplay::clear() {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i)
		_entries[i].active = false;
}

int MadsTextDisplay::add(int xp, int yp, uint fontColour, int charSpacing, const char *msg, Font *font) {
	int usedSlot = -1;

	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		if (!_entries[idx].active) {
			usedSlot = idx;

			_entries[idx].bounds.left = xp;
			_entries[idx].bounds.top = yp;
			_entries[idx].font = font;
			_entries[idx].msg = msg;
			_entries[idx].bounds.setWidth(font->getWidth(msg, charSpacing));
			_entries[idx].bounds.setHeight(font->getHeight());
			_entries[idx].colour1 = fontColour & 0xff;
			_entries[idx].colour2 = fontColour >> 8;
			_entries[idx].spacing = charSpacing;
			_entries[idx].expire = 1;
			_entries[idx].active = true;
			break;
		}
	}

	return usedSlot;
}

void MadsTextDisplay::setDirtyAreas() {
	// Determine dirty areas for active text areas
	for (uint idx = 0, dirtyIdx = DIRTY_AREAS_TEXT_DISPLAY_IDX; dirtyIdx < DIRTY_AREAS_SIZE; ++idx, ++dirtyIdx) {
		if ((_entries[idx].expire < 0) || !_entries[idx].active)
			_owner._dirtyAreas[dirtyIdx].active = false;
		else {
			_owner._dirtyAreas[dirtyIdx].textActive = true;
			_owner._dirtyAreas.setTextDisplay(dirtyIdx, _entries[idx]);
		}
	}
}

void MadsTextDisplay::setDirtyAreas2() {
	// Determine dirty areas for active text areas
	for (uint idx = 0, dirtyIdx = DIRTY_AREAS_TEXT_DISPLAY_IDX; dirtyIdx < DIRTY_AREAS_SIZE; ++idx, ++dirtyIdx) {
		if (_entries[idx].active && (_entries[idx].expire >= 0)) {
			_owner._dirtyAreas.setTextDisplay(dirtyIdx, _entries[idx]);
			_owner._dirtyAreas[dirtyIdx].textActive = (_entries[idx].expire <= 0) ? 0 : 1;
		}
	}
}

void MadsTextDisplay::draw(View *view) {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].active && (_entries[idx].expire >= 0)) {
			_entries[idx].font->setColours(_entries[idx].colour1, _entries[idx].colour2, 0);
			_entries[idx].font->writeString(view, _entries[idx].msg, 
				_entries[idx].bounds.left, _entries[idx].bounds.top, _entries[idx].bounds.width(),
				_entries[idx].spacing);
		}
	}

	// Clear up any now text display entries that are to be expired
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].expire < 0) {
			_entries[idx].active = false;
			_entries[idx].expire = 0;
		}
	}
}

/**
 * Deactivates any text display entries that are finished
 */
void MadsTextDisplay::cleanUp() {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].expire < 0) {
			_entries[idx].active = false;
			_entries[idx].expire = 0;
		}
	}
}

//--------------------------------------------------------------------------

MadsKernelMessageList::MadsKernelMessageList(MadsView &owner): _owner(owner) {
	for (int i = 0; i < TIMED_TEXT_SIZE; ++i) {
		MadsKernelMessageEntry rec;
		_entries.push_back(rec);
	}

	_owner._textSpacing = -1;
	_talkFont = _vm->_font->getFont(FONT_CONVERSATION_MADS);
	word_8469E = 0;
}

void MadsKernelMessageList::clear() {
	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i].flags = 0;

	_owner._textSpacing = -1;
	_talkFont = _vm->_font->getFont(FONT_CONVERSATION_MADS);
}

int MadsKernelMessageList::add(const Common::Point &pt, uint fontColour, uint8 flags, uint8 v2, uint32 timeout, const char *msg) {
	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && ((_entries[idx].flags & KMSG_ACTIVE) != 0))
		++idx;
	if (idx == _entries.size()) {
		if (v2 == 0)
			return -1;

		error("MadsKernelList overflow");
	}

	MadsKernelMessageEntry &rec = _entries[idx];
	strcpy(rec.msg, msg);
	rec.flags = flags | KMSG_ACTIVE;
	rec.colour1 = fontColour & 0xff;
	rec.colour2 = fontColour >> 8;
	rec.position = pt;
	rec.textDisplayIndex = -1;
	rec.timeout = timeout;
	rec.frameTimer = _madsVm->_currentTimer;
	rec.field_1C = v2;
	rec.abortMode = _owner._abortTimersMode2;
	
	for (int i = 0; i < 3; ++i)
		rec.actionNouns[i] = _madsVm->scene()->actionNouns[i];

	if (flags & KMSG_2)
		rec.frameTimer = _owner._ticksAmount + _owner._newTimeout;

	return idx;
}

int MadsKernelMessageList::addQuote(int quoteId, int v2, uint32 timeout) {
	const char *quoteStr = _madsVm->globals()->getQuote(quoteId);
	return add(Common::Point(0, 0), 0x1110, KMSG_2 | KMSG_20, v2, timeout, quoteStr);
}

void MadsKernelMessageList::unk1(int msgIndex, int numTicks, int v2) {
	if (msgIndex < 0)
		return;

	_entries[msgIndex].flags |= (v2 == 0) ? KMSG_8 : (KMSG_8 | KMSG_1);
	_entries[msgIndex].msgOffset = 0;
	_entries[msgIndex].numTicks = numTicks;
	_entries[msgIndex].frameTimer2 = _madsVm->_currentTimer;

	const char *msgP = _entries[msgIndex].msg;
	_entries[msgIndex].asciiChar = *msgP;
	_entries[msgIndex].asciiChar2 = *(msgP + 1);

	if (_entries[msgIndex].flags & KMSG_2)
		_entries[msgIndex].frameTimer2 = _owner._ticksAmount + _owner._newTimeout;

	_entries[msgIndex].frameTimer = _entries[msgIndex].frameTimer2;
}

void MadsKernelMessageList::setSeqIndex(int msgIndex, int seqIndex) {
	if (msgIndex >= 0) {
		_entries[msgIndex].flags |= KMSG_4;
		_entries[msgIndex].sequenceIndex = seqIndex;
	}
}

void MadsKernelMessageList::remove(int msgIndex) {
	MadsKernelMessageEntry &rec = _entries[msgIndex];

	if (rec.flags & KMSG_ACTIVE) {
		if (rec.flags & KMSG_8) {
			//*(rec.msg + rec.msgOffset) = rec.asciiChar;
			//*(rec.msg + rec.msgOffset + 1) = rec.asciiChar2;
		}

		if (rec.textDisplayIndex >= 0)
			_owner._textDisplay.expire(rec.textDisplayIndex);

		rec.flags &= ~KMSG_ACTIVE;
	}
}

void MadsKernelMessageList::reset() {
	for (uint i = 0; i < _entries.size(); ++i)
		remove(i);

	// sub_20454
}

void MadsKernelMessageList::update() {
	uint32 currentTimer = _madsVm->_currentTimer;

	for (uint i = 0; i < _entries.size(); ++i) {
		if (((_entries[i].flags & KMSG_ACTIVE) != 0) && (currentTimer >= _entries[i].frameTimer))
			processText(i);
	}
}

void MadsKernelMessageList::processText(int msgIndex) {
	MadsKernelMessageEntry &msg = _entries[msgIndex];
	uint32 currentTimer = _madsVm->_currentTimer;
	bool flag = false;

	if ((msg.flags & KMSG_40) != 0) {
		_owner._textDisplay.expire(msg.textDisplayIndex);
		msg.flags &= !KMSG_ACTIVE;
		return;
	}

	if ((msg.flags & KMSG_8) == 0) {
		msg.timeout -= 3;
	}

	if (msg.flags & KMSG_4) {
		MadsSequenceEntry &seqEntry = _owner._sequenceList[msg.sequenceIndex];
		if (seqEntry.doneFlag || !seqEntry.active)
			msg.timeout = 0;
	}

	if ((msg.timeout <= 0) && (_owner._abortTimers == 0)) {
		msg.flags |= KMSG_40;
		if (msg.field_1C != 0) {
			_owner._abortTimers = msg.field_1C;
			_owner._abortTimersMode = msg.abortMode;

			if (_owner._abortTimersMode != ABORTMODE_1) {
				for (int i = 0; i < 3; ++i)
					_madsVm->scene()->actionNouns[i] = msg.actionNouns[i];
			}
		}
	}

	msg.frameTimer = currentTimer + 3;
	int x1 = 0, y1 = 0;

	if (msg.flags & KMSG_4) {
		MadsSequenceEntry &seqEntry = _owner._sequenceList[msg.sequenceIndex];
		if (seqEntry.field_12) {
			SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(seqEntry.spriteListIndex);
			M4Sprite *frame = spriteSet.getFrame(seqEntry.frameIndex - 1);
			x1 = frame->bounds().left;
			y1 = frame->bounds().top;
		} else {
			x1 = seqEntry.msgPos.x;
			y1 = seqEntry.msgPos.y;
		}
	}
	
	if (msg.flags & KMSG_2) {
		if (word_8469E != 0) {
			// TODO: Figure out various flags
		} else {
			x1 = 160;
			y1 = 78;
		}
	}

	x1 += msg.position.x;
	y1 += msg.position.y;

	if ((msg.flags & KMSG_8) && (msg.frameTimer >= currentTimer)) {
		msg.msg[msg.msgOffset] = msg.asciiChar;
		char *msgP = &msg.msg[++msg.msgOffset];
		*msgP = msg.asciiChar2;
		
		msg.asciiChar = *msgP;
		msg.asciiChar2 = *(msgP + 1);

		if (!msg.asciiChar) {
			*msgP = '\0';
			msg.flags &= ~KMSG_8;
		} else if (msg.flags & KMSG_1) {
			*msgP = '"';
			*(msgP + 1) = '\0';
		}

		msg.frameTimer = msg.frameTimer2 = currentTimer + msg.numTicks;
		flag = true;
	}

	int strWidth = _talkFont->getWidth(msg.msg, _owner._textSpacing); 

	if (msg.flags & KMSG_30) {
		x1 -= (msg.flags & KMSG_20) ? strWidth / 2 : strWidth;
	}

	// Make sure text appears entirely on-screen
	int x2 = x1 + strWidth;
	if (x2 > MADS_SURFACE_WIDTH)
		x1 -= x2 - MADS_SURFACE_WIDTH;
	if (x1 > (MADS_SURFACE_WIDTH - 1))
		x1 = MADS_SURFACE_WIDTH - 1;
	if (x1 < 0)
		x1 = 0;

	if (y1 > (MADS_SURFACE_HEIGHT - 1))
		y1 = MADS_SURFACE_HEIGHT - 1;
	if (y1 < 0)
		y1 = 0;

	if (msg.textDisplayIndex >= 0) {
		MadsTextDisplayEntry textEntry = _owner._textDisplay[msg.textDisplayIndex];

		if (flag || (textEntry.bounds.left != x1) || (textEntry.bounds.top != y1)) {
			// Mark the associated text entry as deleted, so it can be re-created
			_owner._textDisplay.expire(msg.textDisplayIndex);
			msg.textDisplayIndex = -1;
		}
	}

	if (msg.textDisplayIndex < 0) {
		// Need to create a new text display entry for this message
		int idx = _owner._textDisplay.add(x1, y1, msg.colour1 | (msg.colour2 << 8), _owner._textSpacing, msg.msg, _talkFont);
		if (idx >= 0)
			msg.textDisplayIndex = idx;
	}
}

//--------------------------------------------------------------------------

/**
 * Clears the entries list
 */
void ScreenObjects::clear() {
	_entries.clear();
}

/**
 * Adds a new entry to the list of screen objects
 */
void ScreenObjects::add(const Common::Rect &bounds, int layer, int idx, int category) {
	ScreenObjectEntry rec;
	rec.bounds = bounds;
	rec.layer = layer;
	rec.index = idx;
	rec.category = category;
	rec.active = true;

	_entries.push_back(rec);
}

/**
 * Scans the list for an element that contains the given mode. The result will be 1 based for a match,
 * with 0 indicating no entry was found
 */
int ScreenObjects::scan(int xp, int yp, int layer) {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].active && _entries[i].bounds.contains(xp, yp) && (_entries[i].layer == layer))
			return i + 1;
	}

	// Entry not found
	return 0;
}

int ScreenObjects::scanBackwards(int xp, int yp, int layer) {
	for (int i = (int)_entries.size() - 1; i >= 0; --i) {
		if (_entries[i].active && _entries[i].bounds.contains(xp, yp) && (_entries[i].layer == layer))
			return i + 1;
	}

	// Entry not found
	return 0;
}

void ScreenObjects::setActive(int category, int idx, bool active) {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].active && (_entries[i].category == category) && (_entries[i].index == idx))
			_entries[i].active = active;
	}
}

/*--------------------------------------------------------------------------*/

MadsDynamicHotspots::MadsDynamicHotspots(MadsView &owner): _owner(owner) {
	for (int i = 0; i < DYNAMIC_HOTSPOTS_SIZE; ++i) {
		DynamicHotspot rec;
		rec.active = false;
		_entries.push_back(rec);
	}
	_flag = true;
	_count = 0;
}

int MadsDynamicHotspots::add(int descId, int field14, int seqIndex, const Common::Rect &bounds) {
	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && _entries[idx].active)
		++idx;
	if (idx == _entries.size())
		error("MadsDynamicHotspots overflow");

	_entries[idx].active = true;
	_entries[idx].descId = descId;
	_entries[idx].seqIndex = seqIndex;
	_entries[idx].bounds = bounds;
	_entries[idx].pos.x = -3;
	_entries[idx].pos.y = 0;
	_entries[idx].facing = 5;
	_entries[idx].field_14 = field14;
	_entries[idx].articleNumber = 6;
	_entries[idx].field_17 = 0;

	++_count;
	_flag = true;

	if (seqIndex >= 0)
		_owner._sequenceList[seqIndex].dynamicHotspotIndex = idx;

	return idx;
}

int MadsDynamicHotspots::setPosition(int index, int xp, int yp, int facing) {
	if (index >= 0) {
		_entries[index].pos.x = xp;
		_entries[index].pos.y = yp;
		_entries[index].facing = facing;
	}

	return index;
}

int MadsDynamicHotspots::set17(int index, int v) {
	if (index >= 0)
		_entries[index].field_17 = v;

	return index;
}

void MadsDynamicHotspots::remove(int index) {
	if (_entries[index].active) {
		if (_entries[index].seqIndex >= 0)
			_owner._sequenceList[_entries[index].seqIndex].dynamicHotspotIndex = -1;
		_entries[index].active = false;

		--_count;
		_flag = true;
	}
}

void MadsDynamicHotspots::reset() {
	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i].active = false;

	_count = 0;
	_flag = false;
}

/*--------------------------------------------------------------------------*/

void MadsDirtyArea::setArea(int width, int height, int maxWidth, int maxHeight) {
	if (bounds.left % 2) {
		--bounds.left;
		++width;
	}
	int right = bounds.left + width;
	if (bounds.left < 0)
		bounds.left = 0;
	if (right < 0)
		right = 0;
	if (right > maxWidth)
		right = maxWidth;

	bounds.right = right;
	bounds2.left = bounds.width() / 2;
	bounds2.right = bounds.left + (bounds.width() + 1) / 2 - 1;

	if (bounds.top < 0)
		bounds.top = 0;
	int bottom = bounds.top + height;
	if (bottom < 0)
		bottom = 0;
	if (bottom > maxHeight)
		bottom = maxHeight;

	bounds.bottom = bottom;
	bounds2.top = bounds.height() / 2;
	bounds2.bottom = bounds.top + (bounds.height() + 1) / 2 - 1;

	active = true;
}

/*--------------------------------------------------------------------------*/

MadsDirtyAreas::MadsDirtyAreas(MadsView &owner): _owner(owner) {
	for (int i = 0; i < DIRTY_AREAS_SIZE; ++i) {
		MadsDirtyArea rec;
		rec.active = false;
		_entries.push_back(rec);
	}
}

void MadsDirtyAreas::setSpriteSlot(int dirtyIdx, const MadsSpriteSlot &spriteSlot) {
	int width, height;
	MadsDirtyArea &dirtyArea = _entries[dirtyIdx];

	if (spriteSlot.spriteType == FULL_SCREEN_REFRESH) {
		// Special entry to refresh the entire screen
		dirtyArea.bounds.left = 0;
		dirtyArea.bounds.top = 0;
		width = MADS_SURFACE_WIDTH;
		height = MADS_SURFACE_HEIGHT;
	} else {
		// Standard sprite slots
		dirtyArea.bounds.left = spriteSlot.xp - _owner._posAdjust.x;
		dirtyArea.bounds.top = spriteSlot.yp - _owner._posAdjust.y;

		SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(spriteSlot.spriteListIndex);
		M4Sprite *frame = spriteSet.getFrame(((spriteSlot.frameNumber & 0x7fff) - 1) & 0x7f);
		
		if (spriteSlot.scale == -1) {
			width = frame->width();
			height = frame->height();
		} else {
			width = frame->width() * spriteSlot.scale / 100;
			height = frame->height() * spriteSlot.scale / 100;

			dirtyArea.bounds.left -= width / 2;
			dirtyArea.bounds.top += -(height - 1);
		}
	}

	dirtyArea.setArea(width, height, MADS_SURFACE_WIDTH, MADS_SURFACE_HEIGHT);
}

void MadsDirtyAreas::setTextDisplay(int dirtyIdx, const MadsTextDisplayEntry &textDisplay) {
	MadsDirtyArea &dirtyArea = _entries[dirtyIdx];
	dirtyArea.bounds.left = textDisplay.bounds.left;
	dirtyArea.bounds.top = textDisplay.bounds.top;

	dirtyArea.setArea(textDisplay.bounds.width(), textDisplay.bounds.height(), MADS_SURFACE_WIDTH, MADS_SURFACE_HEIGHT);
}

/**
 * Merge together any designated dirty areas that overlap
 * @param startIndex	1-based starting dirty area starting index
 * @param count			Number of entries to process
 */
void MadsDirtyAreas::merge(int startIndex, int count) {
	if (startIndex >= count)
		return;

	for (int outerCtr = startIndex - 1, idx = 0; idx < count; ++outerCtr, ++idx) {
		if (!_entries[outerCtr].active)
			continue;

		for (int innerCtr = outerCtr + 1; innerCtr < count; ++innerCtr) {
			if (!_entries[innerCtr].active || !intersects(outerCtr, innerCtr))
				continue;

			if (_entries[outerCtr].textActive && _entries[innerCtr].textActive)
				mergeAreas(outerCtr, innerCtr);
		}
	}
}

/**
 * Returns true if two dirty areas intersect
 */
bool MadsDirtyAreas::intersects(int idx1, int idx2) {
	return _entries[idx1].bounds2.intersects(_entries[idx2].bounds2);
}

void MadsDirtyAreas::mergeAreas(int idx1, int idx2) {
	MadsDirtyArea &da1 = _entries[idx1];
	MadsDirtyArea &da2 = _entries[idx2];

	da1.bounds.extend(da2.bounds);

	da1.bounds2.left = da1.bounds.width() / 2;
	da1.bounds2.right = da1.bounds.left + (da1.bounds.width() + 1) / 2 - 1;
	da1.bounds2.top = da1.bounds.height() / 2;
	da1.bounds2.bottom = da1.bounds.top + (da1.bounds.height() + 1) / 2 - 1;

	da2.active = false;
	da1.textActive = true;
}

void MadsDirtyAreas::copy(M4Surface *dest, M4Surface *src) {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].active && _entries[i].bounds.isValidRect())
			src->copyTo(dest, _entries[i].bounds, _entries[i].bounds.left, _entries[i].bounds.top);
	}
}

/*--------------------------------------------------------------------------*/

MadsSequenceList::MadsSequenceList(MadsView &owner): _owner(owner) {
	for (int i = 0; i < TIMER_LIST_SIZE; ++i) {
		MadsSequenceEntry rec;
		rec.active = 0;
		rec.dynamicHotspotIndex = -1;
		_entries.push_back(rec);
	}
}

void MadsSequenceList::clear() {
	for (uint i = 0; i < _entries.size(); ++i) {
		_entries[i].active = 0;
		_entries[i].dynamicHotspotIndex = -1;
	}
}

bool MadsSequenceList::addSubEntry(int index, SequenceSubEntryMode mode, int frameIndex, int abortVal) {
	if (_entries[index].entries.count >= TIMER_ENTRY_SUBSET_MAX)
		return true;

	int subIndex = _entries[index].entries.count++;
	_entries[index].entries.mode[subIndex] = mode;
	_entries[index].entries.frameIndex[subIndex] = frameIndex;
	_entries[index].entries.abortVal[subIndex] = abortVal;

	return false;
}

int MadsSequenceList::add(int spriteListIndex, int v0, int frameIndex, int triggerCountdown, int delayTicks, int extraTicks, int numTicks, 
		int msgX, int msgY, char field_12, char scale, uint8 depth, int frameInc, SpriteAnimType animType, int numSprites, 
		int frameStart) {

	// Find a free slot
	uint timerIndex = 0;
	while ((timerIndex < _entries.size()) && (_entries[timerIndex].active))
		++timerIndex;
	if (timerIndex == _entries.size())
		error("TimerList full");

	if (frameStart <= 0)
		frameStart = 1;
	if (numSprites == 0)
		numSprites = _madsVm->scene()->_spriteSlots.getSprite(spriteListIndex).getCount();
	if (frameStart == numSprites)
		frameInc = 0;

	// Set the list entry fields
	_entries[timerIndex].active = true;
	_entries[timerIndex].spriteListIndex = spriteListIndex;
	_entries[timerIndex].field_2 = v0;
	_entries[timerIndex].frameIndex = frameIndex;
	_entries[timerIndex].frameStart = frameStart;
	_entries[timerIndex].numSprites = numSprites;
	_entries[timerIndex].animType = animType;
	_entries[timerIndex].frameInc = frameInc;
	_entries[timerIndex].depth = depth;
	_entries[timerIndex].scale = scale;
	_entries[timerIndex].field_12 = field_12;
	_entries[timerIndex].msgPos.x = msgX;
	_entries[timerIndex].msgPos.y = msgY;
	_entries[timerIndex].numTicks = numTicks;
	_entries[timerIndex].extraTicks = extraTicks;

	_entries[timerIndex].timeout = _madsVm->_currentTimer + delayTicks;

	_entries[timerIndex].triggerCountdown = triggerCountdown;
	_entries[timerIndex].doneFlag = false;
	_entries[timerIndex].field_13 = 0;
	_entries[timerIndex].dynamicHotspotIndex = -1;
	_entries[timerIndex].entries.count = 0;
	_entries[timerIndex].abortMode = _owner._abortTimersMode2;

	for (int i = 0; i < 3; ++i)
		_entries[timerIndex].actionNouns[i] = _madsVm->scene()->actionNouns[i];

	return timerIndex;
}

void MadsSequenceList::remove(int timerIndex) {
	if (_entries[timerIndex].active) {
		if (_entries[timerIndex].dynamicHotspotIndex >= 0)
			_owner._dynamicHotspots.remove(_entries[timerIndex].dynamicHotspotIndex);
	}

	_entries[timerIndex].active = false;
	_owner._spriteSlots.deleteTimer(timerIndex);
}

void MadsSequenceList::setSpriteSlot(int timerIndex, MadsSpriteSlot &spriteSlot) {
	MadsSequenceEntry &timerEntry = _entries[timerIndex];
	SpriteAsset &sprite = _owner._spriteSlots.getSprite(timerEntry.spriteListIndex);

	spriteSlot.spriteType = sprite.getAssetType() == 1 ? BACKGROUND_SPRITE : FOREGROUND_SPRITE;
	spriteSlot.timerIndex = timerIndex;
	spriteSlot.spriteListIndex = timerEntry.spriteListIndex;
	spriteSlot.frameNumber = ((timerEntry.field_2 == 1) ? 0x8000 : 0) | timerEntry.frameIndex;
	spriteSlot.depth = timerEntry.depth;
	spriteSlot.scale = timerEntry.scale;
	
	if (timerEntry.field_12 == 0) {
		spriteSlot.xp = timerEntry.msgPos.x;
		spriteSlot.yp = timerEntry.msgPos.y;
	} else {
		spriteSlot.xp = sprite.getFrame(timerEntry.frameIndex - 1)->x;
		spriteSlot.yp = sprite.getFrame(timerEntry.frameIndex - 1)->y;
	}
}

bool MadsSequenceList::loadSprites(int timerIndex) {
	MadsSequenceEntry &seqEntry = _entries[timerIndex];
	int slotIndex;
	bool result = false;
	int idx = -1;

	_owner._spriteSlots.deleteTimer(timerIndex);
	if (seqEntry.doneFlag) {
		remove(timerIndex);
		return false;
	}

	if (seqEntry.spriteListIndex == -1) {
		// Doesn't have an associated sprite anymore, so mark as done
		seqEntry.doneFlag = true;
	} else if ((slotIndex = _owner._spriteSlots.getIndex()) >= 0) {
		MadsSpriteSlot &spriteSlot = _owner._spriteSlots[slotIndex];
		setSpriteSlot(timerIndex, spriteSlot);

		int x2 = 0, y2 = 0;

		if ((seqEntry.field_13 != 0) || (seqEntry.dynamicHotspotIndex >= 0)) {
			SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(seqEntry.spriteListIndex);
			M4Sprite *frame = spriteSet.getFrame(seqEntry.frameIndex - 1);
			int width = frame->width() * seqEntry.scale / 200;
			int height = frame->height() * seqEntry.scale / 100;

			warning("frame size %d x %d", width, height);

			// TODO: Missing stuff here, and I'm not certain about the dynamic hotspot stuff below

			if (seqEntry.dynamicHotspotIndex >= 0) {
				DynamicHotspot &dynHotspot = _owner._dynamicHotspots[seqEntry.dynamicHotspotIndex];

				dynHotspot.bounds.left = MAX(x2 - width, 0);
				dynHotspot.bounds.right = MAX(x2 - width, 319) - dynHotspot.bounds.left + 1;
				dynHotspot.bounds.top = MAX(y2 - height, 0);
				dynHotspot.bounds.bottom = MIN(y2, 155) - dynHotspot.bounds.top;

				_owner._dynamicHotspots._flag = true;
			}
		}

		// Frame adjustments
		if (seqEntry.frameStart != seqEntry.numSprites)
			seqEntry.frameIndex += seqEntry.frameInc;

		if (seqEntry.frameIndex >= seqEntry.frameStart) {
			if (seqEntry.frameIndex > seqEntry.numSprites) {
				result = true;
				if (seqEntry.animType == ANIMTYPE_CYCLED) {
					// Reset back to the starting frame (cyclic)
					seqEntry.frameIndex = seqEntry.frameStart;
				} else {
					// Switch into reverse mode
					seqEntry.frameIndex = seqEntry.numSprites - 1;
					seqEntry.frameInc = -1;
				}
			}
		} else {
			// Currently in reverse mode and moved past starting frame
			result = true;

			if (seqEntry.animType == ANIMTYPE_CYCLED)
			{
				// Switch back to forward direction again
				seqEntry.frameIndex = seqEntry.frameStart + 1;
				seqEntry.frameInc = 1;
			} else {
				// Otherwise reset back to last sprite for further reverse animating
				seqEntry.frameIndex = seqEntry.numSprites;		
			}
		}

		if (result && (seqEntry.triggerCountdown != 0)) {
			if (--seqEntry.triggerCountdown == 0)
				seqEntry.doneFlag = true;
		}
	} else {
		// Out of sprite display slots, so mark entry as done
		seqEntry.doneFlag = true;
	}

	if (seqEntry.entries.count > 0) {
		for (int i = 0; i <= seqEntry.entries.count; ++i) {
			switch (seqEntry.entries.mode[i]) {
			case SM_0:
			case SM_1:
				if (((seqEntry.entries.mode[i] == SM_0) && seqEntry.doneFlag) ||
					((seqEntry.entries.mode[i] == SM_1) && result))
				idx = i;
				break;

			case SM_FRAME_INDEX: {
				int v = seqEntry.entries.frameIndex[i];
				if ((v == seqEntry.frameIndex) || (v == 0))
					idx = i;
			}

			default:
				break;
			}
		}
	}

	if (idx >= 0) {
		_owner._abortTimers = seqEntry.entries.abortVal[idx];
		_owner._abortTimersMode = seqEntry.abortMode;
	}

	return result;
}

/**
 * Handles counting down entries in the timer list for action
 */
void MadsSequenceList::tick() {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if ((_owner._abortTimers2 == 0) && (_owner._abortTimers != 0))
			break;

		MadsSequenceEntry &seqEntry = _entries[idx];
		uint32 currentTimer = _madsVm->_currentTimer;

		if (!seqEntry.active || (currentTimer < seqEntry.timeout))
			continue;

		// Set the next timeout for the timer entry
		seqEntry.timeout = currentTimer + seqEntry.numTicks;		

		// Action the sprite
		if (loadSprites(idx)) {
			seqEntry.timeout += seqEntry.extraTicks;
		}
	}
}

void MadsSequenceList::delay(uint32 v1, uint32 v2) {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].active) {
			_entries[idx].timeout += v1 - v2;
		}
	}
}

void MadsSequenceList::setAnimRange(int timerIndex, int startVal, int endVal) {
	MadsSequenceEntry &seqEntry = _entries[timerIndex];
	SpriteAsset &spriteSet = _owner._spriteSlots.getSprite(seqEntry.spriteListIndex);
	int numSprites = spriteSet.getCount();
	int tempStart = startVal, tempEnd = endVal;

	switch (startVal) {
	case -2:
		tempStart = numSprites;
		break;
	case -1:
		tempStart = 1;
		break;
	}
	
	switch (endVal) {
	case -2:
	case 0:
		tempEnd = numSprites;
		break;
	case -1:
		tempEnd = 1;
		break;
	default:
		tempEnd = numSprites;
		break;
	}

	seqEntry.frameStart = tempStart;
	seqEntry.numSprites = tempEnd;

	seqEntry.frameIndex = (seqEntry.frameInc < 0) ? tempStart : tempEnd;
}

//--------------------------------------------------------------------------

MadsView::MadsView(View *view): _view(view), _dynamicHotspots(*this), _sequenceList(*this),
		_kernelMessages(*this), _spriteSlots(*this), _dirtyAreas(*this), _textDisplay(*this) {
	_textSpacing = -1;
	_ticksAmount = 3;
	_newTimeout = 0;
	_abortTimers = 0;
	_abortTimers2 = 0;
	_abortTimersMode = ABORTMODE_0;
	_abortTimersMode2 = ABORTMODE_0;
	
	_depthSurface = NULL;
	_bgSurface = NULL;
}

void MadsView::refresh() {
	// Draw any sprites
	_spriteSlots.drawBackground();

	// Process dirty areas
	_textDisplay.setDirtyAreas();

	// Merge any identified dirty areas
	_dirtyAreas.merge(1, DIRTY_AREAS_SIZE);
	
	// Copy dirty areas to the main display surface 
	_dirtyAreas.copy(_view, _bgSurface);

	// Handle dirty areas for foreground objects
	_spriteSlots.setDirtyAreas();
	_textDisplay.setDirtyAreas2();
	_dirtyAreas.merge(1, DIRTY_AREAS_SIZE);

	// Draw foreground sprites
	_spriteSlots.drawForeground(_view);

	// Draw text elements onto the view
	_textDisplay.draw(_view);

	// Remove any sprite slots that are no longer needed
	_spriteSlots.cleanUp();

	// Deactivate any text display entries that are no longer needed
	_textDisplay.cleanUp();
}

/*--------------------------------------------------------------------------
 * MadsInterfaceView handles the user interface section at the bottom of
 * game screens in MADS games
 *--------------------------------------------------------------------------
 */

MadsInterfaceView::MadsInterfaceView(MadsM4Engine *vm): GameInterfaceView(vm, 
		Common::Rect(0, MADS_SURFACE_HEIGHT, vm->_screen->width(), vm->_screen->height())) {
	_screenType = VIEWID_INTERFACE;
	_highlightedElement = -1;
	_topIndex = 0;
	_selectedObject = -1;
	_cheatKeyCtr = 0;

	_objectSprites = NULL;
	_objectPalData = NULL;

	/* Set up the rect list for screen elements */
	// Actions
	for (int i = 0; i < 10; ++i)
		_screenObjects.addRect((i / 5) * 32 + 1, (i % 5) * 8 + MADS_SURFACE_HEIGHT + 2,
			((i / 5) + 1) * 32 + 3, ((i % 5) + 1) * 8 + MADS_SURFACE_HEIGHT + 2);

	// Scroller elements (up arrow, scroller, down arrow)
	_screenObjects.addRect(73, 160, 82, 167);
	_screenObjects.addRect(73, 168, 82, 190);
	_screenObjects.addRect(73, 191, 82, 198);

	// Inventory object names
	for (int i = 0; i < 5; ++i)
		_screenObjects.addRect(89, 158 + i * 8, 160, 166 + i * 8);

	// Full rectangle area for all vocab actions
	for (int i = 0; i < 5; ++i)
		_screenObjects.addRect(239, 158 + i * 8, 320, 166 + i * 8);
}

MadsInterfaceView::~MadsInterfaceView() {
	delete _objectSprites;
}

void MadsInterfaceView::setFontMode(InterfaceFontMode newMode) {
	switch (newMode) {
	case ITEM_NORMAL:
		_vm->_font->current()->setColours(4, 4, 0xff);
		break;
	case ITEM_HIGHLIGHTED:
		_vm->_font->current()->setColours(5, 5, 0xff);
		break;
	case ITEM_SELECTED:
		_vm->_font->current()->setColours(6, 6, 0xff);
		break;
	}
}

void MadsInterfaceView::initialise() {
	// Build up the inventory list
	_inventoryList.clear();

	for (uint i = 0; i < _madsVm->globals()->getObjectsSize(); ++i) {
		MadsObject *obj = _madsVm->globals()->getObject(i);
		if (obj->roomNumber == PLAYER_INVENTORY)
			_inventoryList.push_back(i);
	}

	// If the inventory has at least one object, select it
	if (_inventoryList.size() > 0)
		setSelectedObject(_inventoryList[0]);
}

void MadsInterfaceView::setSelectedObject(int objectNumber) {
	char resName[80];

	// Load inventory resource
	if (_objectSprites) {
		_vm->_palette->deleteRange(_objectPalData);
		delete _objectSprites;
	}

	// Check to make sure the object is in the inventory, and also visible on-screen
	int idx = _inventoryList.indexOf(objectNumber);
	if (idx == -1) {
		// Object wasn't found, so return
		_selectedObject = -1;
		return;
	}

	// Found the object
	if (idx < _topIndex)
		_topIndex = idx;
	else if (idx >= (_topIndex + 5))
		_topIndex = MAX(0, idx - 4);

	_selectedObject = objectNumber;
	sprintf(resName, "*OB%.3dI.SS", objectNumber);

	Common::SeekableReadStream *data = _vm->res()->get(resName);
	_objectSprites = new SpriteAsset(_vm, data, data->size(), resName);
	_vm->res()->toss(resName);

	// Slot it into available palette space
	_objectPalData = _objectSprites->getRgbList();
	_vm->_palette->addRange(_objectPalData);
	_objectSprites->translate(_objectPalData, true);

	_objectFrameNumber = 0;
}

void MadsInterfaceView::addObjectToInventory(int objectNumber) {
	if (_inventoryList.indexOf(objectNumber) == -1) {
		_madsVm->globals()->getObject(objectNumber)->roomNumber = PLAYER_INVENTORY;
		_inventoryList.push_back(objectNumber);
	}

	setSelectedObject(objectNumber);
}

void MadsInterfaceView::onRefresh(RectList *rects, M4Surface *destSurface) {
	_vm->_font->setFont(FONT_INTERFACE_MADS);
	char buffer[100];

	// Check to see if any dialog is currently active
	bool dialogVisible = _vm->_viewManager->getView(LAYER_DIALOG) != NULL;

	// Highlighting logic for action list
	int actionIndex = 0;
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 5; ++y, ++actionIndex) {
			// Determine the font colour depending on whether an item is selected. Note that the first action,
			// 'Look', is always 'selected', even when another action is clicked on
			setFontMode((_highlightedElement == actionIndex) ? ITEM_HIGHLIGHTED :
				((actionIndex == 0) ? ITEM_SELECTED : ITEM_NORMAL));

			// Get the verb action and capitalise it
			const char *verbStr = _madsVm->globals()->getVocab(kVerbLook + actionIndex);
			strcpy(buffer, verbStr);
			if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

			// Display the verb
			const Common::Rect r(_screenObjects[actionIndex]);
			_vm->_font->current()->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
		}
	}

	// Check for highlighting of the scrollbar controls
	if ((_highlightedElement == SCROLL_UP) || (_highlightedElement == SCROLL_SCROLLER) || (_highlightedElement == SCROLL_DOWN)) {
		// Highlight the control's borders
		const Common::Rect r(_screenObjects[_highlightedElement]);
		destSurface->frameRect(r, 5);
	}

	// Draw the horizontal line in the scroller representing the current top selected
	const Common::Rect scroller(_screenObjects[SCROLL_SCROLLER]);
	int yP = (_inventoryList.size() < 2) ? 0 : (scroller.height() - 5) * _topIndex / (_inventoryList.size() - 1);
	destSurface->setColor(4);
	destSurface->hLine(scroller.left + 2, scroller.right - 3, scroller.top + 2 + yP);

	// List inventory items
	for (uint i = 0; i < 5; ++i) {
		if ((_topIndex + i) >= _inventoryList.size())
			break;

		const char *descStr = _madsVm->globals()->getVocab(_madsVm->globals()->getObject(
			_inventoryList[_topIndex + i])->descId);
		strcpy(buffer, descStr);
		if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

		const Common::Rect r(_screenObjects[INVLIST_START + i]);

		// Set the highlighting of the inventory item
		if (_highlightedElement == (int)(INVLIST_START + i)) setFontMode(ITEM_HIGHLIGHTED);
		else if (_selectedObject == _inventoryList[_topIndex + i]) setFontMode(ITEM_SELECTED);
		else setFontMode(ITEM_NORMAL);

		// Write out it's description
		_vm->_font->current()->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
	}

	// Handle the display of any currently selected object
	if (_objectSprites) {
		// Display object sprite. Note that the frame number isn't used directly, because it would result
		// in too fast an animation
		M4Sprite *spr = _objectSprites->getFrame(_objectFrameNumber / INV_ANIM_FRAME_SPEED);
		spr->copyTo(destSurface, INVENTORY_X, INVENTORY_Y, 0);

		if (!_madsVm->globals()->_config.invObjectsStill && !dialogVisible) {
			// If objects need to be animated, move to the next frame
			if (++_objectFrameNumber >= (_objectSprites->getCount() * INV_ANIM_FRAME_SPEED))
				_objectFrameNumber = 0;
		}

		// List the vocab actions for the currently selected object
		MadsObject *obj = _madsVm->globals()->getObject(_selectedObject);
		int yIndex = MIN(_highlightedElement - VOCAB_START, obj->vocabCount - 1);

		for (int i = 0; i < obj->vocabCount; ++i) {
			const Common::Rect r(_screenObjects[VOCAB_START + i]);

			// Get the vocab description and capitalise it
			const char *descStr = _madsVm->globals()->getVocab(obj->vocabList[i].vocabId);
			strcpy(buffer, descStr);
			if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

			// Set the highlighting and display the entry
			setFontMode((i == yIndex) ? ITEM_HIGHLIGHTED : ITEM_NORMAL);
			_vm->_font->current()->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
		}
	}
}

bool MadsInterfaceView::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	MadsAction &act = _madsVm->scene()->getAction();

	// If the mouse isn't being held down, then reset the repeated scroll timer
	if (eventType != MEVENT_LEFT_HOLD)
		_nextScrollerTicks = 0;

	// Handle various event types
	switch (eventType) {
	case MEVENT_MOVE:
		// If the cursor isn't in "wait mode", don't do any processing
		if (_vm->_mouse->getCursorNum() == CURSOR_WAIT)
			return true;

		// Ensure the cursor is the standard arrow
		_vm->_mouse->setCursorNum(CURSOR_ARROW);

		// Check if any interface element is currently highlighted
		_highlightedElement = _screenObjects.find(Common::Point(x, y));

		return true;

	case MEVENT_LEFT_CLICK:
		// Left mouse click
		{
			// Check if an inventory object was selected
			if ((_highlightedElement >= INVLIST_START) && (_highlightedElement < (INVLIST_START + 5))) {
				// Ensure there is an inventory item listed in that cell
				uint idx = _highlightedElement - INVLIST_START;
				if ((_topIndex + idx) < _inventoryList.size()) {
					// Set the selected object
					setSelectedObject(_inventoryList[_topIndex + idx]);
				}
			} else if ((_highlightedElement >= ACTIONS_START) && (_highlightedElement < (ACTIONS_START + 10))) {
				// A standard action was selected
				int verbId = kVerbLook + (_highlightedElement - ACTIONS_START);
				warning("Selected action #%d", verbId);
				
			} else if ((_highlightedElement >= VOCAB_START) && (_highlightedElement < (VOCAB_START + 5))) {
				// A vocab action was selected
				MadsObject *obj = _madsVm->globals()->getObject(_selectedObject);
				int vocabIndex = MIN(_highlightedElement - VOCAB_START, obj->vocabCount - 1);
				if (vocabIndex >= 0) {
					act._actionMode = ACTMODE_OBJECT;
					act._actionMode2 = ACTMODE2_2;
					act._flags1 = obj->vocabList[1].flags1;
					act._flags2 = obj->vocabList[1].flags2;

					act._currentHotspot = _selectedObject;
					act._articleNumber = act._flags2;
				}
			}
		}
		return true;

	case MEVENT_LEFT_HOLD:
		// Left mouse hold
		// Handle the scroller - the up/down buttons allow for multiple actions whilst the mouse is held down
		if ((_highlightedElement == SCROLL_UP) || (_highlightedElement == SCROLL_DOWN)) {
			if ((_nextScrollerTicks == 0) || (g_system->getMillis() >= _nextScrollerTicks)) {
				// Handle scroll up/down action
				_nextScrollerTicks = g_system->getMillis() + SCROLLER_DELAY;

				if ((_highlightedElement == SCROLL_UP) && (_topIndex > 0))
					--_topIndex;
				if ((_highlightedElement == SCROLL_DOWN) && (_topIndex < (int)(_inventoryList.size() - 1)))
					++_topIndex;
			}
		}
		return true;

	case MEVENT_LEFT_DRAG:
		// Left mouse drag
		// Handle the the the scroller area that can be dragged to adjust the top displayed index
		if (_highlightedElement == SCROLL_SCROLLER) {
			// Calculate the new top index based on the Y position
			const Common::Rect r(_screenObjects[SCROLL_SCROLLER]);
			_topIndex = CLIP((int)(_inventoryList.size() - 1) * (y - r.top - 2) / (r.height() - 5),
				0, (int)_inventoryList.size() - 1);
		}
		return true;

	case KEVENT_KEY:
		if (_cheatKeyCtr == CHEAT_SEQUENCE_MAX)
			handleCheatKey(param1);
		handleKeypress(param1);
		return true;

	default:
		break;
	}

	return false;
}

bool MadsInterfaceView::handleCheatKey(int32 keycode) {
	switch (keycode) {
	case Common::KEYCODE_SPACE:
		// TODO: Move player to current destination
		return true;

	case Common::KEYCODE_t | (Common::KEYCODE_LALT):
	case Common::KEYCODE_t | (Common::KEYCODE_RALT):
	{
		// Teleport to room
		//Scene *sceneView = (Scene *)vm->_viewManager->getView(VIEWID_SCENE);


		return true;
	}

	default:
		break;
	}

	return false;
}

const char *CHEAT_SEQUENCE = "widepipe";

bool MadsInterfaceView::handleKeypress(int32 keycode) {
	int flags = keycode >> 24;
	int kc = keycode & 0xffff;

	// Capitalise the letter if necessary
	if (_cheatKeyCtr < CHEAT_SEQUENCE_MAX) {
		if ((flags & Common::KBD_CTRL) && (kc == CHEAT_SEQUENCE[_cheatKeyCtr])) {
			++_cheatKeyCtr;
			if (_cheatKeyCtr == CHEAT_SEQUENCE_MAX)
				Dialog::display(_vm, 22, cheatingEnabledDesc);
			return true;
		} else {
			_cheatKeyCtr = 0;
		}
	}

	// Handle the various keys
	if ((keycode == Common::KEYCODE_ESCAPE) || (keycode == Common::KEYCODE_F1)) {
		// Game menu
		_madsVm->globals()->dialogType = DIALOG_GAME_MENU;
		leaveScene();
		return false;
	} else if (flags & Common::KBD_CTRL) {
		// Handling of the different control key combinations
		switch (kc) {
		case Common::KEYCODE_i:
			// Mouse to inventory
			warning("TODO: Mouse to inventory");
			break;

		case Common::KEYCODE_k:
			// Toggle hotspots
			warning("TODO: Toggle hotspots");
			break;

		case Common::KEYCODE_p:
			// Player stats
			warning("TODO: Player stats");
			break;

		case Common::KEYCODE_q:
			// Quit game
			break;

		case Common::KEYCODE_s:
			// Activate sound
			warning("TODO: Activate sound");
			break;

		case Common::KEYCODE_u:
			// Rotate player
			warning("TODO: Rotate player");
			break;

		case Common::KEYCODE_v: {
			// Release version
			Dialog *dlg = new Dialog(_vm, GameReleaseInfoStr, GameReleaseTitleStr);
			_vm->_viewManager->addView(dlg);
			_vm->_viewManager->moveToFront(dlg);
			return false;
		}

		default:
			break;
		}
	} else if ((flags & Common::KBD_ALT) && (kc == Common::KEYCODE_q)) {
		// Quit Game

	} else {
		// Standard keypresses
		switch (kc) {
			case Common::KEYCODE_F2:
				// Save game
				_madsVm->globals()->dialogType = DIALOG_SAVE;
				leaveScene();
				break;
			case Common::KEYCODE_F3:
				// Restore game
				_madsVm->globals()->dialogType = DIALOG_RESTORE;
				leaveScene();
				break;
		}
	}
//DIALOG_OPTIONS
	return false;
}

void MadsInterfaceView::leaveScene() {
	// Close the scene
	View *view = _madsVm->_viewManager->getView(VIEWID_SCENE);
	_madsVm->_viewManager->deleteView(view);
}

} // End of namespace M4
