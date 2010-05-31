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

#include "m4/assets.h"
#include "m4/animation.h"
#include "m4/compression.h"

namespace M4 {

// TODO: this code needs cleanup

MadsAnimation::MadsAnimation(MadsM4Engine *vm, MadsView *view): Animation(vm), _view(view) {
	_playing = false;
	_font = NULL;
	_unk1 = 0;
	_skipLoad = false;
	_unkIndex = -1;
	_messageCtr= 0;
}

MadsAnimation::~MadsAnimation() {
	delete _font;
}

void MadsAnimation::load(const Common::String &filename) {
	MadsPack anim(filename.c_str(), _vm);
	bool madsRes = filename[0] == '*';
	char buffer[20];
	int streamIndex = 1;

	// Chunk 1: header
	// header

	Common::SeekableReadStream *animStream = anim.getItemStream(0);

	int spriteListCount = animStream->readUint16LE();
	int miscEntriesCount = animStream->readUint16LE();
	int frameEntryCount = animStream->readUint16LE();
	int messagesCount = animStream->readUint16LE();
	animStream->skip(1);
	_flags = animStream->readByte();

	animStream->skip(2);
	_animMode = animStream->readUint16LE();
	assert(_animMode != 4);
	_roomNumber = animStream->readUint16LE();
	_field12 = animStream->readUint16LE() != 0;
	animStream->skip(4);
	_spriteListIndex = animStream->readUint16LE();
	_scrollX = animStream->readUint16LE();
	_scrollY = animStream->readSint16LE();
	animStream->skip(10);
	
	animStream->read(buffer, 13);
	_field24 = Common::String(buffer, 13);

	for (int i = 0; i < 10; ++i) {
		animStream->read(buffer, 13);
		_spriteSetNames[i] = Common::String(buffer, 13);
	}

	animStream->skip(81);
	animStream->read(buffer, 13);
	_lbmFilename = Common::String(buffer, 13);
	animStream->read(buffer, 13);
	_spritesFilename = Common::String(buffer, 13);
	animStream->skip(48);
	animStream->read(buffer, 13);
	_soundName = Common::String(buffer, 13);
	animStream->skip(26);
	animStream->read(buffer, 13);
	Common::String fontResource(buffer, 13);

	// TODO: Based on a weird usage of a flags word, a secondary method gets called here.
	// Figure out secondary method, and when/if it's called

	// Initialise the reference list
	for (int i = 0; i < spriteListCount; ++i)
		_spriteListIndexes.push_back(-1);

	delete animStream;

	if (messagesCount > 0) {
		// Chunk 2
		// Following is a list of any messages for the animation

		animStream = anim.getItemStream(streamIndex++);

		for (int i = 0; i < messagesCount; ++i) {
			AnimMessage rec;
			animStream->read(rec.msg, 70);
			rec.pos.x = animStream->readUint16LE();
			rec.pos.y = animStream->readUint16LE();
			animStream->readUint16LE();
			rec.rgb1.r = animStream->readByte();
			rec.rgb1.g = animStream->readByte();
			rec.rgb1.b = animStream->readByte();
			rec.rgb2.r = animStream->readByte();
			rec.rgb2.g = animStream->readByte();
			rec.rgb2.b = animStream->readByte();
			rec.kernelMsgIndex = animStream->readUint16LE();
			animStream->skip(6);
			rec.startFrame = animStream->readUint16LE();
			rec.endFrame = animStream->readUint16LE();
			animStream->readUint16LE();

			_messages.push_back(rec);
		}

		delete animStream;
	}

	if (frameEntryCount > 0) {
		// Chunk 3: animation frame info
		animStream = anim.getItemStream(streamIndex++);

		for (int i = 0; i < frameEntryCount; i++) {
			AnimFrameEntry rec;
			rec.frameNumber = animStream->readUint16LE();
			rec.seqIndex = animStream->readByte();
			rec.spriteSlot.frameNumber = animStream->readUint16LE();
			rec.spriteSlot.xp = animStream->readUint16LE();
			rec.spriteSlot.yp = animStream->readUint16LE();
			rec.spriteSlot.depth = animStream->readUint16LE();
			rec.spriteSlot.scale = animStream->readUint16LE();

			_frameEntries.push_back(rec);
		}

		delete animStream;
	}

	if (miscEntriesCount > 0) {
		// Chunk 4: Misc Data
		animStream = anim.getItemStream(streamIndex);

		for (int i = 0; i < miscEntriesCount; ++i) {
			AnimMiscEntry rec;
			rec.soundNum = animStream->readUint16LE();
			rec.numTicks = animStream->readUint16LE();
			rec.posAdjust.x = animStream->readUint16LE();
			rec.posAdjust.y = animStream->readUint16LE();
			animStream->readUint16LE();

			_miscEntries.push_back(rec);
		}

		delete animStream;
	}

	// If the animation specifies a font, then load it for access
	if (_flags & ANIM_CUSTOM_FONT) {
		Common::String fontName;
		if (madsRes)
			fontName += "*";
		fontName += fontResource;

		_font = _vm->_font->getFont(fontName);
	}

	// Load all the sprite sets for the animation
	for (int i = 0; i < spriteListCount; ++i) {
		_spriteListIndexes[i] = _view->_spriteSlots.addSprites(_spriteSetNames[i].c_str());
	}


	if (_field12) {
		Common::String resName;
		if (madsRes)
			resName += "*";
		resName += _spriteSetNames[_spriteListIndex];
		
		_spriteListIndexes[_spriteListIndex] = _view->_spriteSlots.addSprites(resName.c_str());
	}

	// TODO: Unknown section about handling palette entries - I think it's adjusting sprite sets
	// to the palette of the game screen

	// Process the sprite list indexes to remap them to the actual sprite list indexes
	
}

void MadsAnimation::start() {
	_currentFrame = 0;
	_oldFrameEntry = 0;
	//for (int i = 0; i < _seriesCount; i++) {
		//_spriteSeries[i] = new SpriteSeries((char*)_spriteSeriesNames[i].c_str());
	//}
	_playing = true;
	update();
}

bool MadsAnimation::update() {
	if (!_playing)
		return true;

	if (_field12) {
		int spriteListIndex = _spriteListIndexes[_spriteListIndex];
		int newIndex = -1;
		
		for (uint idx = _oldFrameEntry; idx < _frameEntries.size(); ++idx) {
			if (_frameEntries[idx].frameNumber > _currentFrame)
				break;
			if (_frameEntries[idx].spriteSlot.spriteListIndex == spriteListIndex)
				newIndex = _frameEntries[idx].spriteSlot.frameNumber;
		}

		if (newIndex >= 0)
			load1(newIndex);
	}

	// If it's not time for the next frame, then exit
	if (_madsVm->_currentTimer < _nextFrameTimer)
		return false;

	// Loop checks for any prior animation sprite slots to be expired
	for (int slotIndex = 0; slotIndex < _view->_spriteSlots.startIndex; ++slotIndex) {
		if ((_view->_spriteSlots[slotIndex].seqIndex >= 0x80) &&
			(_view->_spriteSlots[slotIndex].seqIndex <= 0xFD)) {
			// Flag the frame as animation sprite slot
			_view->_spriteSlots[slotIndex].spriteType = EXPIRED_SPRITE;
		}
	}

	// Validate the current frame
	if (_currentFrame > (int)_miscEntries.size()) {
		// Is the animation allowed to be repeated?
		if (_resetFlag) {
			_currentFrame = 0;
			_oldFrameEntry = 0;
		} else {
			_unk1 = true;
			return true;
		}
	}

	// Handle starting any sound for this frame
	AnimMiscEntry &misc = _miscEntries[_currentFrame];
	if (misc.soundNum)
		_vm->_sound->playSound(misc.soundNum);

	bool screenChanged = false;

	// Handle any scrolling of the screen surface
	if ((_scrollX != 0) || (_scrollY != 0)) {
		_view->_bgSurface->scrollX(_scrollX);
		_view->_bgSurface->scrollY(_scrollY);

		screenChanged = true;
	}

	// Handle any offset adjustment for sprites as of this frame
	if (_view->_posAdjust.x != misc.posAdjust.x) {
		misc.posAdjust.x = _view->_posAdjust.x;
		screenChanged = true;
	}
	if (_view->_posAdjust.y != misc.posAdjust.y) {
		misc.posAdjust.y = _view->_posAdjust.y;
		screenChanged = true;
	}
	if (screenChanged) {
		// Signal the entire screen needs refreshing
		_view->_spriteSlots.fullRefresh();
	}

	int spriteSlotsMax = _view->_spriteSlots.startIndex;

	// Main frame animation loop - frames get animated by being placed, as necessary, into the
	// main sprite slot array
	while ((uint)_oldFrameEntry < _frameEntries.size()) {
		if (_frameEntries[_oldFrameEntry].frameNumber > _currentFrame)
			break;
		else if (_frameEntries[_oldFrameEntry].frameNumber == _currentFrame) {
			// Found the correct frame 
			int spriteSlotIndex = 0;
			int index = 0;

			for (;;) {
				if ((spriteSlotIndex == 0) && (index < spriteSlotsMax)) {
					int seqIndex = _frameEntries[_oldFrameEntry].seqIndex - _view->_spriteSlots[index].seqIndex;
					if (seqIndex == 0x80) {
						if (_view->_spriteSlots[index] == _frameEntries[_oldFrameEntry].spriteSlot)
							_view->_spriteSlots[index].spriteType = SPRITE_ZERO;
					}
					++index;
					continue;
				} 
				
				if (spriteSlotIndex == 0) {
					int slotIndex = _view->_spriteSlots.getIndex();
					_view->_spriteSlots[slotIndex].copy(_frameEntries[_oldFrameEntry].spriteSlot);
					_view->_spriteSlots[slotIndex].seqIndex += 0x80;
					
					SpriteAsset &spriteSet = _view->_spriteSlots.getSprite(
						_view->_spriteSlots[slotIndex].spriteListIndex);

					_view->_spriteSlots[slotIndex].spriteType = (spriteSet.getAssetType() == 0) ?
						SPRITE_FOUR : SPRITE_ZERO;
				}
				break;
			}
		}
		
		++_oldFrameEntry;
	}

	// Handle the display of any messages
	for (uint idx = 0; idx < _messages.size(); ++idx) {
		if (_messages[idx].kernelMsgIndex >= 0) {
			// Handle currently active message
			if ((_currentFrame < _messages[idx].startFrame) || (_currentFrame > _messages[idx].endFrame)) {
				_view->_kernelMessages.remove(_messages[idx].kernelMsgIndex);
				_messages[idx].kernelMsgIndex = -1;
				--_messageCtr;
			}
		} else if ((_currentFrame >= _messages[idx].startFrame) && (_currentFrame <= _messages[idx].endFrame)) {
			// Start displaying the message
			AnimMessage &me = _messages[idx];

			// The colour index to use is dependant on how many messages are currently on-screen
			uint8 colIndex;
			switch (_messageCtr) {
			case 1:
				colIndex = 252;
				break;
			case 2:
				colIndex = 16;
				break;
			default:
				colIndex = 250;
				break;
			}

			_vm->_palette->setEntry(colIndex, me.rgb1.r, me.rgb1.g, me.rgb1.b);
			_vm->_palette->setEntry(colIndex + 1, me.rgb2.r, me.rgb2.g, me.rgb2.b);

			// Add a kernel message to display the given text
			me.kernelMsgIndex = _view->_kernelMessages.add(me.pos, colIndex * 101, 0, 0, INDEFINITE_TIMEOUT, me.msg);
			++_messageCtr;
		}
	}

	// Move to the next frame
	_currentFrame++;
	if (_currentFrame >= (int)_miscEntries.size()) {
		// Animation is complete
		stop();

		if (_abortTimers != 0) {
			_view->_abortTimers = _abortTimers;
			_view->_abortTimersMode = _abortMode;

			if (_abortMode != ABORTMODE_1) {
				// Copy the noun list
				for (int i = 0; i < 3; ++i)
					_madsVm->scene()->actionNouns[i] = _actionNouns[i];
			}
		}
	}

	int frameNum = MIN(_currentFrame, (int)_miscEntries.size() - 1);
	_nextFrameTimer = _madsVm->_currentTimer + _miscEntries[frameNum].numTicks;

	return _currentFrame >= (int)_miscEntries.size();
}

void MadsAnimation::stop() {
	_playing = false;
}

void MadsAnimation::setCurrentFrame(int frameNumber) {
	_currentFrame = frameNumber;
	_oldFrameEntry = 0;
	_unk1 = 0;
}

void MadsAnimation::load1(int frameNumber) {
	if (_skipLoad)
		return;

	Common::Point pt;
	int listIndex = _spriteListIndexes[_spriteListIndex];
	SpriteAsset &spriteSet = _view->_spriteSlots.getSprite(listIndex);

	if (_unkIndex < 0) {
		M4Surface *frame = spriteSet.getFrame(0);
		pt.x = frame->bounds().left;
		pt.y = frame->bounds().top;
	} else {
		pt.x = _unkList[_unkIndex].x;
		pt.y = _unkList[_unkIndex].y;
		_unkIndex = 1 - _unkIndex;
	}

	if (proc1(spriteSet, pt, frameNumber))
		error("proc1 failure");


}

bool MadsAnimation::proc1(SpriteAsset &spriteSet, const Common::Point &pt, int frameNumber) {
	return 0;
}

} // End of namespace M4
