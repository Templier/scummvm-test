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

#include "common/system.h"

#include "sound/audiostream.h"
#include "sound/wave.h"

#include "tucker/tucker.h"
#include "tucker/graphics.h"

namespace Tucker {

void TuckerEngine::handleIntroSequence() {
	const int firstSequence = (_gameFlags & kGameFlagDemo) != 0 ? kFirstAnimationSequenceDemo : kFirstAnimationSequenceGame;
	_player = new AnimationSequencePlayer(_system, _mixer, _eventMan, firstSequence);
	_player->mainLoop();
	delete _player;
	_player = 0;
}

void TuckerEngine::handleCreditsSequence() {
	static const int _creditsSequenceData1[] = { 200, 350, 650, 850, 1150, 1450, 12000 };
	static const int _creditsSequenceData2[] = { 1, 1, 5, 0, 6, 6, 0 };
	int counter4 = 0;
	int counter3 = 0;
	int num = 0;
	int imgNum = 0;
	int prevLocationNum = _locationNum;
	int counter2 = 0;
	int counter1 = 0;
	loadCharset2();
	stopSounds();
	_locationNum = 74;
	_flagsTable[236] = 74;
	uint8 *imgBuf = (uint8 *)malloc(16 * 64000);
	loadSprC02_01();
	_spritesCount = _creditsSequenceData2[num];
	for (int i = 0; i < _spritesCount; ++i) {
		memset(&_spritesTable[i], 0, sizeof(Sprite));
		_spritesTable[i].state = -1;
		_spritesTable[i].stateIndex = -1;
	}
	loadFile("credits.txt", _ptTextBuf);
	loadImage("loc74.pcx", _quadBackgroundGfxBuf, 1);
	startSpeechSound(9001, 120);
	_timerCounter2 = 0;
	_fadePaletteCounter = 0;
	do {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		if (counter4 + 20 > _creditsSequenceData1[num]) {
			fadeInPalette();
		}
		++imgNum;
		if (imgNum == 16) {
			imgNum = 0;
		}
		if (num < 6) {
			Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf, 320, 320, 200);
		} else {
			Graphics::copyTo640(_locationBackgroundGfxBuf, imgBuf + imgNum * 64000, 320, 320, 200);
			drawCreditsString(5, 48, counter2 * 6);
			drawCreditsString(5, 60, counter2 * 6 + 1);
			drawCreditsString(5, 80, counter2 * 6 + 2);
			drawCreditsString(5, 92, counter2 * 6 + 3);
			drawCreditsString(5, 140, counter2 * 6 + 4);
			drawCreditsString(5, 116, counter2 * 6 + 5);
			++counter1;
			if (counter1 < 20) {
				fadePaletteColor(191, kFadePaletteStep);
			} else if (counter1 > 106) {
				fadePaletteColor(191, -kFadePaletteStep);
			}
			if (counter1 > 116) {
				counter1 = 0;
			}
			++counter2;
			if (counter2 > 17) {
				counter2 = 0;
			}
		}
		_fullRedrawCounter = 2;
		++counter3;
		if (counter3 == 2) {
			counter3 = 0;
			updateSprites();
		}
		for (int i = 0; i < _spritesCount; ++i) {
			drawSprite(i);
		}
		redrawScreen(0);
		waitForTimer(3);
		_timerCounter1 = 0;
		counter4 = _timerCounter2 / 3;
		if (counter4 == _creditsSequenceData1[num]) {
			_fadePaletteCounter = 0;
			_spritesCount = _creditsSequenceData2[num];
			for (int i = 0; i < _spritesCount; ++i) {
				memset(&_spritesTable[i], 0, sizeof(Sprite));
				_spritesTable[i].state = -1;
				_spritesTable[i].stateIndex = -1;
			}
			++num;
			char filename[40];
			if (num == 6) {
				for (int i = 0; i < 16; ++i) {
					sprintf(filename, "cogs%04d.pcx", i);
					loadImage(filename, imgBuf + i * 64000, 2);
				}
			} else {
				switch (num) {
				case 1:
					strcpy(filename, "loc75.pcx");
					break;
				case 2:
					strcpy(filename, "loc76.pcx");
					break;
				case 3:
					strcpy(filename, "paper-3.pcx");
					break;
				case 4:
					strcpy(filename, "loc77.pcx");
					break;
				case 5:
					strcpy(filename, "loc78.pcx");
					break;
				}
				loadImage(filename, _quadBackgroundGfxBuf, 2);
			}
			_spritesCount = _creditsSequenceData2[num];
			++_flagsTable[236];
		}
	} while (!_quitGame && isSpeechSoundPlaying());
	free(imgBuf);
	_locationNum = prevLocationNum;
	do {
		if (_fadePaletteCounter > 0) {
			fadeInPalette();
			--_fadePaletteCounter;
		}
		redrawScreen(0);
		waitForTimer(2);
	} while (_fadePaletteCounter > 0);
}

void TuckerEngine::handleCongratulationsSequence() {
	_timerCounter2 = 0;
	_fadePaletteCounter = 0;
	stopSounds();
	loadImage("congrat.pcx", _loadTempBuf, 1);
	Graphics::copyTo640(_locationBackgroundGfxBuf, _loadTempBuf, 320, 320, 200);
	_fullRedrawCounter = 2;
	redrawScreen(0);
	while (!_quitGame && _timerCounter2 < 450) {
		while (_fadePaletteCounter < 14) {
			++_fadePaletteCounter;
			fadeOutPalette();
		}
		waitForTimer(3);
	}
}

void TuckerEngine::handleNewPartSequence() {
	char filename[40];

	stopSounds();
	if (_flagsTable[219] == 1) {
		_flagsTable[219] = 0;
		for (int i = 0; i < 50; ++i) {
			_inventoryItemsState[i] = 0;
		}
		_inventoryObjectsOffset = 0;
		_inventoryObjectsCount = 0;
		addObjectToInventory(30);
		if (_partNum == 1 || _partNum == 3) {
			addObjectToInventory(1);
			addObjectToInventory(0);
		}
		_redrawPanelItemsCounter = 0;
	}
	_scrollOffset = 0;
	switch (_partNum) {
	case 1:
		strcpy(filename, "pt1bak.pcx");
		break;
	case 2:
		strcpy(filename, "pt2bak.pcx");
		break;
	default:
		strcpy(filename, "pt3bak.pcx");
		break;
	}
	loadImage(filename, _quadBackgroundGfxBuf, 1);
	_spritesCount = 1;
	memset(&_spritesTable[0], 0, sizeof(Sprite));
	_spritesTable[0].state = -1;
	_spritesTable[0].stateIndex = -1;
	int currentLocation = _locationNum;
	_locationNum = 98;
	unloadSprA02_01();
	unloadSprC02_01();
	switch (_partNum) {
	case 1:
		strcpy(filename, "sprites/partone.spr");
		break;
	case 2:
		strcpy(filename, "sprites/parttwo.spr");
		break;
	default:
		strcpy(filename, "sprites/partthr.spr");
		break;
	}
	_sprC02Table[1] = loadFile(filename, 0);
	startSpeechSound(9000, 60);
	_fadePaletteCounter = 0;
	do {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf, 320, 320, 200);
		_fullRedrawCounter = 2;
		updateSprites();
		drawSprite(0);
		redrawScreen(0);
		waitForTimer(3);
		if (_inputKeys[kInputKeyEscape]) {
			_inputKeys[kInputKeyEscape] = false;
			break;
		}
	} while (isSpeechSoundPlaying());
	stopSpeechSound();
	do {
		if (_fadePaletteCounter > 0) {
			fadeInPalette();
			--_fadePaletteCounter;
		}
		Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf, 320, 320, 200);
		_fullRedrawCounter = 2;
		updateSprites();
		drawSprite(0);
		redrawScreen(0);
		waitForTimer(3);
	} while (_fadePaletteCounter > 0);
	_locationNum = currentLocation;
}

void TuckerEngine::handleMeanwhileSequence() {
	char filename[40];
	uint8 backupPalette[256 * 3];
	memcpy(backupPalette, _currentPalette, 256 * 3);
	switch (_partNum) {
	case 1:
		strcpy(filename, "meanw01.pcx");
		break;
	case 2:
		strcpy(filename, "meanw02.pcx");
		break;
	default:
		strcpy(filename, "meanw03.pcx");
		break;
	}
	if (_flagsTable[215] == 0 && _flagsTable[231] == 1) {
		strcpy(filename, "loc80.pcx");
	}
	loadImage(filename, _quadBackgroundGfxBuf + 89600, 1);
	_fadePaletteCounter = 0;
	for (int i = 0; i < 60; ++i) {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		_fullRedrawCounter = 2;
		redrawScreen(0);
		waitForTimer(3);
		++i;
	}
	do {
		if (_fadePaletteCounter > 0) {
			fadeInPalette();
			--_fadePaletteCounter;
		}
		Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		_fullRedrawCounter = 2;
		redrawScreen(0);
		waitForTimer(3);
	} while (_fadePaletteCounter > 0);
	memcpy(_currentPalette, backupPalette, 256 * 3);
}

void TuckerEngine::handleMapSequence() {
	loadImage("map2.pcx", _quadBackgroundGfxBuf + 89600, 0);
	loadImage("map1.pcx", _loadTempBuf, 1);
	_selectedObject.locationObject_locationNum = 0;
	if (_flagsTable[7] > 0) {
		copyMapRect(0, 0, 140, 86);
	}
	if (_flagsTable[7] > 1) {
		copyMapRect(0, 60, 122, 120);
	}
	if (_flagsTable[7] > 2) {
		copyMapRect(122, 114, 97, 86);
	}
	if (_flagsTable[7] == 4) {
		copyMapRect(140, 0, 88, 125);
	}
	if (_flagsTable[120] == 1) {
		copyMapRect(220, 0, 100, 180);
	}
	_fadePaletteCounter = 0;
	int xPos = 0, yPos = 0, textNum = 0;
	while (!_quitGame) {
		waitForTimer(2);
		updateMouseState();
		Graphics::copyTo640(_locationBackgroundGfxBuf + _scrollOffset, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		_fullRedrawCounter = 2;
		if (_flagsTable[7] > 0 && _mousePosX > 30 && _mousePosX < 86 && _mousePosY > 36 && _mousePosY < 86) {
			textNum = 13;
			_nextLocationNum = (_partNum == 1) ? 3 : 65;
			xPos = 620;
			yPos = 130;
		} else if (_flagsTable[7] > 1 && _mousePosX > 60 && _mousePosX < 120 && _mousePosY > 120 && _mousePosY < 170) {
			textNum = 14;
			_nextLocationNum = (_partNum == 1) ? 9 : 66;
			xPos = 344;
			yPos = 120;
		} else if (_flagsTable[7] > 2 && _mousePosX > 160 && _mousePosX < 210 && _mousePosY > 110 && _mousePosY < 160) {
			textNum = 15;
			_nextLocationNum = (_partNum == 1) ? 16 : 61;
			xPos = 590;
			yPos = 130;
		} else if ((_flagsTable[7] == 4 || _flagsTable[7] == 6) && _mousePosX > 150 && _mousePosX < 200 && _mousePosY > 20 && _mousePosY < 70) {
			textNum = 16;
			_nextLocationNum = (_partNum == 1) ? 20 : 68;
			xPos = 20;
			yPos = 130;
		} else if (_flagsTable[120] == 1 && _mousePosX > 240 && _mousePosX < 290 && _mousePosY > 35 && _mousePosY < 90) {
			textNum = 17;
			_nextLocationNum = (_partNum == 1) ? 19 : 62;
			xPos = 20;
			yPos = 124;
		} else if (_mousePosX > 135 && _mousePosX < 185 && _mousePosY > 170 && _mousePosY < 200) {
			textNum = 18;
			_nextLocationNum = _locationNum;
			if (!_noPositionChangeAfterMap) {
				xPos = _xPosCurrent;
				yPos = _yPosCurrent;
			} else if (_locationNum == 3 || _locationNum == 65) {
				xPos = 620;
				yPos = 130;
			} else if (_locationNum == 9 || _locationNum == 66) {
				xPos = 344;
				yPos = 120;
			} else if (_locationNum == 16 || _locationNum == 61) {
				xPos = 590;
				yPos = 130;
			} else if (_locationNum == 20 || _locationNum == 68) {
				xPos = 20;
				yPos = 130;
			} else {
				xPos = 20;
				yPos = 124;
			}
		}
		if (textNum > 0) {
			drawSpeechText(_scrollOffset + _mousePosX + 8, _mousePosY - 10, _infoBarBuf, textNum, 96);
		}
		redrawScreen(_scrollOffset);
		if (_fadePaletteCounter < 14) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		if (_leftMouseButtonPressed && textNum != 0) {
			break;
		}
	}
	while (_fadePaletteCounter > 0) {
		fadeInPalette();
		redrawScreen(_scrollOffset);
		--_fadePaletteCounter;
	}
	_mouseClick = 1;
	if (_nextLocationNum == 9 && _noPositionChangeAfterMap) {
		_backgroundSpriteCurrentAnimation = 2;
		_backgroundSpriteCurrentFrame = 0;
		setCursorType(2);
	} else if (_nextLocationNum == 66 && _noPositionChangeAfterMap) {
		_backgroundSpriteCurrentAnimation = 1;
		_backgroundSpriteCurrentFrame = 0;
		setCursorType(2);
	}
	_noPositionChangeAfterMap = false;
	_xPosCurrent = xPos;
	_yPosCurrent = yPos;
}

void TuckerEngine::copyMapRect(int x, int y, int w, int h) {
	const uint8 *src = _loadTempBuf + y * 320 + x;
	uint8 *dst = _quadBackgroundGfxBuf + 89600 + y * 320 + x;
	for (int i = 0; i < h; ++i) {
		memcpy(dst, src, w);
		src += 320;
		dst += 320;
	}
}

int TuckerEngine::handleSpecialObjectSelectionSequence() {
	char filename[40];
	if (_partNum == 1 && _selectedObjectNum == 6) {
		strcpy(filename, "news1.pcx");
		_flagsTable[7] = 4;
	} else if (_partNum == 3 && _selectedObjectNum == 45) {
		strcpy(filename, "profnote.pcx");
	} else if (_partNum == 1 && _selectedObjectNum == 26) {
		strcpy(filename, "photo.pcx");
	} else if (_partNum == 3 && _selectedObjectNum == 39) {
		strcpy(filename, "news2.pcx");
		_flagsTable[135] = 1;
	} else if (_currentInfoString1SourceType == 0 && _currentActionObj1Num == 259) {
		strcpy(filename, "postit.pcx");
	} else if (_currentInfoString1SourceType == 1 && _currentActionObj1Num == 91) {
		strcpy(filename, "memo.pcx");
	} else {
		return 0;
	}
	while (_fadePaletteCounter > 0) {
		fadeInPalette();
		redrawScreen(_scrollOffset);
		--_fadePaletteCounter;
	}
	_mouseClick = 1;
	loadImage(filename, _quadBackgroundGfxBuf, 1);
	_fadePaletteCounter = 0;
	while (!_quitGame) {
		waitForTimer(2);
		updateMouseState();
		Graphics::copyTo640(_locationBackgroundGfxBuf + _scrollOffset, _quadBackgroundGfxBuf, 320, 320, 200);
		_fullRedrawCounter = 2;
		if (_fadePaletteCounter < 14) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		if (!_leftMouseButtonPressed && _mouseClick == 1) {
			_mouseClick = 0;
		}
		if (_partNum == 3 && _selectedObjectNum == 45) {
			for (int i = 0; i < 13; ++i) {
				const int offset = _dataTable[204 + i].yDest * 640 + _dataTable[204 + i].xDest;
				static const int itemsTable[] = { 15, 44, 25, 19, 21, 24, 12, 27, 20, 29, 35, 23, 3 };
				if (_inventoryItemsState[itemsTable[i]] > 1) {
					Graphics::decodeRLE(_locationBackgroundGfxBuf + _scrollOffset + offset, _data3GfxBuf + _dataTable[204 + i].sourceOffset, _dataTable[204 + i].xSize, _dataTable[204 + i].ySize);
				}
			}
		}
		redrawScreen(_scrollOffset);
		if (_leftMouseButtonPressed && _mouseClick != 1) {
			while (_fadePaletteCounter > 0) {
				fadeInPalette();
				redrawScreen(_scrollOffset);
				--_fadePaletteCounter;
			}
			_mouseClick = 1;
			break;
		}
	}
	loadLoc();
	return 1;
}

AnimationSequencePlayer::AnimationSequencePlayer(OSystem *system, Audio::Mixer *mixer, Common::EventManager *event, int num)
	: _system(system), _mixer(mixer), _event(event), _seqNum(num), _currentSeqNum(0) {
	_newSeq = false;
	memset(_animationPalette, 0, sizeof(_animationPalette));
	memset(_paletteBuffer, 0, sizeof(_paletteBuffer));
	_soundSeqDataOffset = 0;
	_soundSeqDataCount = 0;
	_soundSeqDataIndex = 0;
	_soundsList1Offset = 0;
	_soundsList1Count = 0;
	_soundsList2Offset = 0;
	_soundsList2Count = 0;
	_musicVolume = 0;
	_offscreenBuffer = (uint8 *)malloc(kScreenWidth * kScreenHeight);
	_updateScreenWidth = 0;
	_updateScreenPicture = 0;
	_updateScreenOffset = 0;
	_frameCounter = 0;
	_frameTime = 1;
	_lastFrameTime = 0;
	_picBufPtr = _pic2BufPtr = 0;
}

AnimationSequencePlayer::~AnimationSequencePlayer() {
	unloadAnimation();
	free(_offscreenBuffer);
}

void AnimationSequencePlayer::mainLoop() {
	do {
		if (_seqNum != _currentSeqNum) {
			unloadAnimation();
			_currentSeqNum = _seqNum;
			_newSeq = true;
			_frameCounter = 0;
			_lastFrameTime = _system->getMillis();
		}
		switch (_seqNum) {
		case 17:
			introSeq17_18();
			break;
		case 19:
			introSeq19_20();
			break;
		case 3:
			introSeq3_4();
			break;
		case 9:
			introSeq9_10();
			break;
		case 21:
			introSeq21_22();
			break;
		case 13:
			introSeq13_14();
			break;
		case 15:
			introSeq15_16();
			break;
		case 27:
			introSeq27_28();
			break;
		}
		_system->copyRectToScreen(_offscreenBuffer, 320, 0, 0, kScreenWidth, kScreenHeight);
		_system->setPalette(_animationPalette, 0, 256);
		_system->updateScreen();
		syncTime();
	} while (_seqNum != 1);
}

void AnimationSequencePlayer::syncTime() {
	uint32 end = _lastFrameTime + kSequenceFrameTime * _frameTime;
	do {
		Common::Event ev;
		while (_event->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_KEYDOWN:
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_seqNum = 1;
				}
				break;
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				_seqNum = 1;
				break;
			default:
				break;
			}
		}
		_system->delayMillis(10);
		_lastFrameTime = _system->getMillis();
	} while (_lastFrameTime <= end);
}

Audio::AudioStream *AnimationSequencePlayer::loadSoundFileAsStream(int index, AnimationSoundType type) {
	const char *name = 0;
	switch (type) {
	case kAnimationSoundType8BitsRAW:
	case kAnimationSoundType16BitsRAW:
		name = _musicFileNamesTable[index];
		break;
	case kAnimationSoundTypeWAV:
	case kAnimationSoundTypeLoopingWAV:
		name = _audioFileNamesTable[index];
		break;
	}
	Audio::AudioStream *stream = 0;
	char fileName[64];
	snprintf(fileName, sizeof(fileName), "audio/%s", name);
	Common::File f;
	if (f.open(fileName)) {
		int size = 0, rate = 0;
		uint8 flags = 0;
		switch (type) {
		case kAnimationSoundType8BitsRAW:
		case kAnimationSoundType16BitsRAW:
			size = f.size();
			rate = 22050;
			flags = Audio::Mixer::FLAG_UNSIGNED;
			if (type == kAnimationSoundType16BitsRAW) {
				flags = Audio::Mixer::FLAG_LITTLE_ENDIAN | Audio::Mixer::FLAG_16BITS;
			}
			break;
		case kAnimationSoundTypeWAV:
		case kAnimationSoundTypeLoopingWAV:
			Audio::loadWAVFromStream(f, size, rate, flags);
			if (type == kAnimationSoundTypeLoopingWAV) {
				flags |= Audio::Mixer::FLAG_LOOP;
			}
			break;
		}
		if (size != 0) {
			uint8 *sampleData = (uint8 *)malloc(size);
			if (sampleData) {
				f.read(sampleData, size);
				flags |= Audio::Mixer::FLAG_AUTOFREE;
				stream = Audio::makeLinearInputStream(sampleData, size, rate, flags, 0, 0);
			}
		}
	}
	return stream;
}

void AnimationSequencePlayer::loadSounds(int type, int num) {
	int index = 0;
	switch (type) {
	case 1:
		switch (num) {
		case 0:
			index = 1;
			_soundsList1Count = 14;
			_soundsList1Offset = 0;
			_soundsList2Count = 10;
			_soundsList2Offset = 14;
			_soundSeqDataCount = 58;
			_soundSeqDataOffset = 0;
			break;
		case 1:
			index = 1;
			_soundsList1Count = 14;
			_soundsList1Offset = 24;
			_soundsList2Count = 5;
			_soundsList2Offset = 38;
			_soundSeqDataCount = 60;
			_soundSeqDataOffset = 58;
			break;
		case 2:
			index = 1;
			_soundsList1Count = 14;
			_soundsList1Offset = 43;
			_soundsList2Count = 9;
			_soundsList2Offset = 57;
			_soundSeqDataCount = 48;
			_soundSeqDataOffset = 118;
			break;
		}
		_musicVolume = 0;
		break;
	case 3:
		switch (num) {
		case 0:
			index = 3;
			_soundsList1Count = 13;
			_soundsList1Offset = 66;
			_soundsList2Count = 0;
			_soundSeqDataCount = 19;
			_soundSeqDataOffset = 166;
			break;
		case 1:
			index = 6;
			_soundsList1Count = 14;
			_soundsList1Offset = 79;
			_soundsList2Count = 4;
			_soundsList2Offset = 93;
			_soundSeqDataCount = 25;
			_soundSeqDataOffset = 185;
			break;
		case 2:
			index = 7;
			_soundsList1Count = 13;
			_soundsList1Offset = 97;
			_soundsList2Count = 9;
			_soundsList2Offset = 110;
			_soundSeqDataCount = 43;
			_soundSeqDataOffset = 210;
			break;
		case 3:
			index = 10;
			_soundsList1Count = 11;
			_soundsList1Offset = 119;
			_soundsList2Count = 0;
			_soundSeqDataCount = 11;
			_soundSeqDataOffset = 253;
			break;
		}
		_musicVolume = 80;
		break;
	case 4:
		switch (num) {
		case 0:
			index = 4;
			_soundsList1Count = 6;
			_soundsList1Offset = 130;
			_soundsList2Count = 0;
			_soundSeqDataCount = 12;
			_soundSeqDataOffset = 264;
			break;
		case 1:
			index = 9;
			_soundsList1Count = 10;
			_soundsList1Offset = 136;
			_soundsList2Count = 0;
			_soundSeqDataCount = 24;
			_soundSeqDataOffset = 276;
			break;
		}
		_musicVolume = 80;
		break;
	case 5:
		index = 5;
		_soundsList1Count = 0;
		_soundsList2Count = 0;
		_soundSeqDataCount = 0;
		_musicVolume = 100;
		break;
	case 9:
		index = 8;
		_soundsList1Count = 0;
		_soundsList2Count = 0;
		_soundSeqDataCount = 0;
		_musicVolume = 100;
		break;
	case 10:
		index = 0;
		_soundsList1Count = 4;
		_soundsList1Offset = 146;
		_soundsList2Count = 0;
		_soundSeqDataCount = 7;
		_soundSeqDataOffset = 300;
		_musicVolume = 100;
		break;
	default:
		index = 0;
		_soundsList1Count = 0;
		_soundsList2Count = 0;
		_soundSeqDataCount = 0;
		_musicVolume = 100;
		break;
	}
	if (_musicVolume != 0) {
		Audio::AudioStream *s;
		if ((s = loadSoundFileAsStream(index, (type == 5) ? kAnimationSoundType16BitsRAW : kAnimationSoundType8BitsRAW)) != 0) {
			_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, scaleMixerVolume(_musicVolume));
		}
	}
	_soundSeqDataIndex = 0;
}

void AnimationSequencePlayer::updateSounds() {
	Audio::AudioStream *s = 0;
	const SoundSequenceData *p = &_soundSeqData[_soundSeqDataOffset + _soundSeqDataIndex];
	while (_soundSeqDataIndex < _soundSeqDataCount && p->timestamp <= _frameCounter) {
		switch (p->opcode) {
		case 0:
			if (p->index < _soundsList1Count) {
				if ((s = loadSoundFileAsStream(_soundsList1Offset + p->index, kAnimationSoundTypeWAV)) != 0) {
					_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_soundsHandle[p->index], s, -1, scaleMixerVolume(p->volume));
				}
			}
			break;
		case 1:
			if (p->index < _soundsList1Count) {
				if ((s = loadSoundFileAsStream(_soundsList1Offset + p->index, kAnimationSoundTypeLoopingWAV)) != 0) {
					_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_soundsHandle[p->index], s, -1, scaleMixerVolume(p->volume));
				}
			}
			break;
		case 2:
			if (p->index < _soundsList1Count) {
				_mixer->stopHandle(_soundsHandle[p->index]);
			}
			break;
		case 3:
			_mixer->stopHandle(_musicHandle);
			break;
		case 4:
			_mixer->stopHandle(_musicHandle);
			if ((s = loadSoundFileAsStream(p->index, kAnimationSoundType8BitsRAW)) != 0) {
				_musicVolume = p->volume;
				_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, scaleMixerVolume(_musicVolume));
			}
			break;
		case 5:
			if (p->index < _soundsList2Count) {
				if ((s = loadSoundFileAsStream(_soundsList2Offset + p->index, kAnimationSoundTypeWAV)) != 0) {
					_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, s, -1, scaleMixerVolume(p->volume));
				}
			}
			break;
		case 6:
			_mixer->stopHandle(_musicHandle);
			if ((s = loadSoundFileAsStream(p->index, kAnimationSoundType16BitsRAW)) != 0) {
				_musicVolume = p->volume;
				_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, scaleMixerVolume(_musicVolume));
			}
			break;
		default:
			warning("Unhandled sound opcode %d (%d,%d)", p->opcode, _frameCounter, p->timestamp);
			break;
		}
		++p;
		++_soundSeqDataIndex;
	}
}

void AnimationSequencePlayer::fadeInPalette() {
	memset(_paletteBuffer, 0, sizeof(_paletteBuffer));
	bool fadeColors = true;
	for (int step = 0; step < 64; ++step) {
		if (fadeColors) {
			fadeColors = false;
			for (int i = 0; i < 1024; ++i) {
				if ((i & 3) != 3 && _paletteBuffer[i] < _animationPalette[i]) {
					const int color = _paletteBuffer[i] + 4;
					_paletteBuffer[i] = MIN<int>(color, _animationPalette[i]);
					fadeColors = true;
				}
			}
			_system->setPalette(_paletteBuffer, 0, 256);
			_system->updateScreen();
		}
		_system->delayMillis(1000 / 60);
	}
}

void AnimationSequencePlayer::fadeOutPalette() {
	memcpy(_paletteBuffer, _animationPalette, 1024);
	bool fadeColors = true;
	for (int step = 0; step < 64; ++step) {
		if (fadeColors) {
			fadeColors = false;
			for (int i = 0; i < 1024; ++i) {
				if ((i & 3) != 3 && _paletteBuffer[i] > 0) {
					const int color = _paletteBuffer[i] - 4;
					_paletteBuffer[i] = MAX<int>(0, color);
					fadeColors = true;
				}
			}
			_system->setPalette(_paletteBuffer, 0, 256);
			_system->updateScreen();
		}
		_system->delayMillis(1000 / 60);
	}
	_system->clearScreen();
}

void AnimationSequencePlayer::unloadAnimation() {
	_mixer->stopAll();
	free(_picBufPtr);
	_picBufPtr = 0;
	free(_pic2BufPtr);
	_pic2BufPtr = 0;
}

uint8 *AnimationSequencePlayer::loadPicture(const char *fileName) {
	uint8 *p = 0;
	Common::File f;
	if (f.open(fileName)) {
		const int sz = f.size();
		p = (uint8 *)malloc(sz);
		if (p) {
			f.read(p, sz);
		}
	}
	return p;
}

void AnimationSequencePlayer::openAnimation(int index, const char *fileName) {
	if (!_flicPlayer[index].loadFile(fileName)) {
		warning("Unable to open flc animation file '%s'", fileName);
		_seqNum = 1;
		return;
	}
	_flicPlayer[index].decodeNextFrame();
	if (index == 0) {
		memcpy(_animationPalette, _flicPlayer[index].getPalette(), 1024);
		memcpy(_offscreenBuffer, _flicPlayer[index].getOffscreen(), kScreenWidth * kScreenHeight);
	}
}

void AnimationSequencePlayer::decodeNextAnimationFrame(int index) {
	_flicPlayer[index].decodeNextFrame();
	memcpy(_offscreenBuffer, _flicPlayer[index].getOffscreen(), kScreenWidth * kScreenHeight);
	if (index == 0) {
		if (_flicPlayer[index].isPaletteDirty()) {
			memcpy(_animationPalette, _flicPlayer[index].getPalette(), 1024);
		}
	}
	if (_seqNum != 19) {
		++_frameCounter;
	}
}

void AnimationSequencePlayer::introSeq17_18() {
	if (_newSeq) {
		loadSounds(9, 0);
		openAnimation(0, "graphics/merit.flc");
		_frameTime = 1;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0].isLastFrame()) {
		_seqNum = 19;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq19_20() {
	if (_newSeq) {
		fadeOutPalette();
		loadSounds(10, 1);
		openAnimation(0, "graphics/budttle2.flc");
		openAnimation(1, "graphics/machine.flc");
		_frameTime = 1;
		_newSeq = false;
	}
	if (_flicPlayer[0].getCurFrame() >= 116) {
		_flicPlayer[1].decodeNextFrame();
		if (_flicPlayer[1].isLastFrame()) {
			_flicPlayer[1].reset();
		}
	}
	_flicPlayer[0].decodeNextFrame();
	const uint8 *t = _flicPlayer[1].getOffscreen();
	for (int i = 0; i < 64000; ++i) {
		const uint8 color = _flicPlayer[0].getOffscreen()[i];
		_offscreenBuffer[i] = color ? color : t[i];
	}
	updateSounds();
	if (_flicPlayer[0].isLastFrame()) {
		_seqNum = 3;
	}
}

void AnimationSequencePlayer::displayLoadingScreen() {
	Common::File f;
	if (f.open("graphics/loading.pic")) {
		fadeOutPalette();
		f.seek(32);
		for (int i = 0; i < 1024; i += 4) {
			f.read(_animationPalette + i, 3);
		}
		f.read(_offscreenBuffer, 64000);
		_system->copyRectToScreen(_offscreenBuffer, 320, 0, 0, kScreenWidth, kScreenHeight);
		fadeInPalette();
	}
}

void AnimationSequencePlayer::initPicPart4() {
	_updateScreenWidth = 320;
	_updateScreenPicture = 1;
	_updateScreenOffset = 0;
}

void AnimationSequencePlayer::drawPicPart4() {
	static const uint8 offsetsTable[77] = {
		1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4,
		5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3,
		3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1
	};
	_updateScreenWidth = _updateScreenWidth - offsetsTable[_updateScreenOffset];
	++_updateScreenOffset;
	for (int y = 0; y < 200; ++y) {
		memcpy(_offscreenBuffer + y * 320, _picBufPtr + 800 + y * 640 + _updateScreenWidth, 320);
	}
	if (_updateScreenWidth == 0) {
		_updateScreenPicture = false;
	}
}

void AnimationSequencePlayer::introSeq3_4() {
	if (_newSeq) {
		displayLoadingScreen();
		loadSounds(1, 0);
		_picBufPtr = loadPicture("graphics/house.pic");
		openAnimation(0, "graphics/intro1.flc");
		_system->copyRectToScreen(_offscreenBuffer, 320, 0, 0, kScreenWidth, kScreenHeight);
		fadeInPalette();
		_updateScreenPicture = false;
		_frameTime = 2;
		_newSeq = false;
	}
	if (!_updateScreenPicture) {
		decodeNextAnimationFrame(0);
		if (_flicPlayer[0].getCurFrame() == 706) {
			initPicPart4();
		}
		if (_flicPlayer[0].isLastFrame()) {
			_seqNum = 9;
		}
	} else {
		drawPicPart4();
	}
	updateSounds();
}

void AnimationSequencePlayer::drawPic2Part10() {
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 64; ++x) {
			const uint8 color = _pic2BufPtr[y * 64 + x];
			if (color != 0) {
				_picBufPtr[89417 + y * 640 + x] = color;
			}
		}
	}
	for (int y = 0; y < 80; ++y) {
		for (int x = 0; x < 48; ++x) {
			const uint8 color = _pic2BufPtr[1024 + y * 48 + x];
			if (color != 0) {
				_picBufPtr[63939 + y * 640 + x] = color;
			}
		}
	}
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 80; ++x) {
			const uint8 color = _pic2BufPtr[7424 + y * 80 + x];
			if (color != 0) {
				_picBufPtr[33067 + y * 640 + x] = color;
			}
		}
	}
}

void AnimationSequencePlayer::drawPic1Part10() {
	for (int y = 0; y < 200; ++y) {
		memcpy(_offscreenBuffer + y * 320, _picBufPtr + 800 + y * 640 + _updateScreenWidth, 320);
	}
	for (int i = 0; i < 64000; ++i) {
		const uint8 color = _flicPlayer[0].getOffscreen()[i];
		if (color) {
			_offscreenBuffer[i] = color;
		}
	}
}

void AnimationSequencePlayer::introSeq9_10() {
	if (_newSeq) {
		loadSounds(1, 1);
		_pic2BufPtr = loadPicture("graphics/bits.pic");
		_picBufPtr = loadPicture("graphics/lab.pic");
		openAnimation(0, "graphics/intro2.flc");
		_updateScreenWidth = 0;
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0].getCurFrame() == 984) {
		drawPic2Part10();
	}
	if (_flicPlayer[0].getCurFrame() >= 264 && _flicPlayer[0].getCurFrame() <= 295) {
		drawPic1Part10();
		_updateScreenWidth += 6;
	} else if (_flicPlayer[0].getCurFrame() >= 988 && _flicPlayer[0].getCurFrame() <= 996) {
		drawPic1Part10();
		_updateScreenWidth -= 25;
		if (_updateScreenWidth < 0) {
			_updateScreenWidth = 0;
		}
	}
	if (_flicPlayer[0].isLastFrame()) {
		_seqNum = 21;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq21_22() {
	if (_newSeq) {
		loadSounds(1, 2);
		openAnimation(0, "graphics/intro3.flc");
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0].isLastFrame()) {
		_seqNum = 1;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq13_14() {
	if (_newSeq) {
		loadSounds(3, 1);
		openAnimation(0, "graphics/allseg02.flc");
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0].isLastFrame()) {
		_seqNum = 15;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq15_16() {
	if (_newSeq) {
		loadSounds(3, 2);
		openAnimation(0, "graphics/allseg03.flc");
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0].isLastFrame()) {
		_seqNum = 27;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq27_28() {
	if (_newSeq) {
		loadSounds(3, 3);
		openAnimation(0, "graphics/allseg04.flc");
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0].isLastFrame()) {
		_seqNum = 1;
	}
	updateSounds();
}

} // namespace Tucker
