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

#include "drascula/drascula.h"

namespace Drascula {

void DrasculaEngine::updateAnim(int y, int destX, int destY, int width, int height, int count, byte* src, int delayVal) {
	int x = 0;

	for (int n = 0; n < count; n++){
		x++;
		copyBackground(x, y, destX, destY, width, height, src, screenSurface);
		updateScreen(destX, destY, destX, destY, width, height, screenSurface);
		x += width;
		pause(delayVal);
	}
}

void DrasculaEngine::updateAnim2(int y, int px, int py, int width, int height, int count, byte* src) {
	int x = 0;

	for (int n = 0; n < count; n++) {
		x++;
		copyBackground(px, py, px, py, width, height, drawSurface1, screenSurface);
		copyRect(x, y, px, py, width, height, src, screenSurface);
		updateScreen(px, py, px, py, width, height, screenSurface);
		x = x + width;
		pause(3);
	}
}

void DrasculaEngine::animation_1_1() {
	int l, l2, p;
	int pixelPos[6];

	while (term_int == 0) {
		playMusic(29);
		playFLI("logoddm.bin", 9);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(600);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		delay(340);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		playMusic(26);
		delay(500);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		playFLI("logoalc.bin", 8);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("cielo.alg", screenSurface, COMPLETE_PAL);
		black();
		updateScreen();
		fadeFromBlack(2);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(900);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		color_abc(kColorRed);
		centerText(_textmisc[_lang][1], 160, 100);
		updateScreen();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(1000);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(1200);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		playFLI("scrollb.bin", 9);

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		playSound(5);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (animate("scr2.bin", 17))
			break;
		stopSound();
		if (animate("scr3.bin", 17))
			break;
		loadPic("cielo2.alg", screenSurface, COMPLETE_PAL);
		updateScreen();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		fadeToBlack(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		loadPic(96, frontSurface, COMPLETE_PAL);
		loadPic(103, drawSurface1, HALF_PAL);
		loadPic(104, drawSurface3);
		loadPic("aux104.alg", drawSurface2);

		playMusic(4);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(400);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		for (l2 = 0; l2 < 3; l2++)
			for (l = 0; l < 7; l++) {
				copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
				copyBackground(interf_x[l], interf_y[l], 156, 45, 63, 31, drawSurface2, screenSurface);
				updateScreen();
				if (getScan() == Common::KEYCODE_ESCAPE) {
					term_int = 1;
					break;
				}
				pause(3);
			}
			if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
				break;

		l2 = 0; p = 0;
		pixelPos[3] = 45;
		pixelPos[4] = 63;
		pixelPos[5] = 31;

		for (l = 0; l < 180; l++) {
			copyBackground(0, 0, 320 - l, 0, l, 200, drawSurface3, screenSurface);
			copyBackground(l, 0, 0, 0, 320 - l, 200, drawSurface1, screenSurface);

			pixelPos[0] = interf_x[l2];
			pixelPos[1] = interf_y[l2];
			pixelPos[2] = 156 - l;

			copyRectClip(pixelPos, drawSurface2, screenSurface);
			updateScreen();
			p++;
			if (p == 6) {
				p = 0;
				l2++;
			}
			if (l2 == 7)
				l2 = 0;
			if (getScan() == Common::KEYCODE_ESCAPE) {
				term_int = 1;
				break;
			}
		}
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		copyBackground(0, 0, 0, 0, 320, 200, screenSurface, drawSurface1);

		talk_dr_grande(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		clearRoom();

		loadPic(100, drawSurface1, HALF_PAL);
		loadPic("auxigor.alg", frontSurface);
		loadPic("auxdr.alg", backSurface);
		trackDrascula = 0;
		x_dr = 129;
		y_dr = 95;
		trackIgor = 1;
		igorX = 66;
		igorY = 97;

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_igor(8, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_drascula(2);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_drascula(3);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (animate("lib.bin", 16))
			break;
		if (animate("lib2.bin", 16))
			break;
		clearRoom();
		color_solo = kColorRed;
		loadPic("plan1.alg", screenSurface, HALF_PAL);
		updateScreen();
		pause(10);
		talk_solo(_textd[_lang][4],"d4.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("plan1.alg", screenSurface, HALF_PAL);
		updateScreen();
		talk_solo(_textd[_lang][5], "d5.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (animate("lib2.bin", 16))
			break;
		clearRoom();
		loadPic("plan2.alg", screenSurface, HALF_PAL);
		updateScreen();
		pause(20);
		talk_solo(_textd[_lang][6], "d6.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (animate("lib2.bin", 16))
			break;
		clearRoom();
		loadPic("plan3.alg", screenSurface, HALF_PAL);
		updateScreen();
		pause(20);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_textd[_lang][7], "d7.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("plan3.alg", screenSurface, HALF_PAL);
		updateScreen();
		talk_solo(_textd[_lang][8], "d8.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic(100, drawSurface1, HALF_PAL);
		MusicFadeout();
		stopMusic();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor(9, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_drascula(9);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor(10, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		playMusic(11);
		talk_drascula(10);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (animate("rayo1.bin", 16))
			break;
		playSound(5);
		if (animate("rayo2.bin", 15))
			break;
		if (animate("frel2.bin", 16))
			break;
		if (animate("frel.bin", 16))
			break;
		if (animate("frel.bin", 16))
			break;
		stopSound();
		clearRoom();
		black();
		playMusic(23);
		fadeFromBlack(0);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		trackDrascula = 1;
		talk_igor(1, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_drascula(11, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		trackDrascula = 3;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
		updateScreen();
		pause(1);
		trackDrascula = 0;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_drascula(12);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		trackDrascula = 3;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
		updateScreen();
		pause(1);
		trackDrascula = 1;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_igor(2, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(13);
		talk_drascula(13, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		trackDrascula = 3;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
		updateScreen();
		pause(1);
		trackDrascula = 0;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
		updateScreen();
		talk_drascula(14);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor(3, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_drascula(15);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor(4, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_drascula(16);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor(5, kIgorDch);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		trackIgor = 3;
		talk_drascula(17);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(18);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor(6, kIgorFront);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		fadeToBlack(0);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		playMusic(2);
		pause(5);
		playFLI("intro.bin", 12);
		term_int = 1;
	}
	clearRoom();
	loadPic(96, frontSurface, COMPLETE_PAL);
	loadPic(99, backSurface);
}

void DrasculaEngine::animation_2_1() {
	int l;

	gotoObject(231, 91);
	hare_se_ve = 0;

	term_int = 0;

	for (;;) {
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		if (animate("ag.bin", 14))
			break;

		if (_lang == kSpanish)
			textSurface = frontSurface;

		loadPic("an11y13.alg", extraSurface);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		talk_bartender(22);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		if (_lang == kSpanish)
			textSurface = extraSurface;

		loadPic(97, extraSurface);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		pause(4);
		playSound(1);
		hiccup(18);
		finishSound();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		clearRoom();
		stopMusic();
		musicStopped = 1;
		memset(screenSurface, 0, 64000);
		color_solo = kColorWhite;
		pause(80);

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_textbj[_lang][1], "BJ1.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("bj.alg", screenSurface, HALF_PAL);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		black();
		updateScreen();
		fadeFromBlack(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = kColorYellow;
		talk_solo(_text[_lang][214], "214.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		loadPic(16, drawSurface1, HALF_PAL);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("auxbj.alg", drawSurface3);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		roomNumber = 16;

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		for (l = 0; l < 200; l++)
			factor_red[l] = 99;
		x_bj = 170;
		y_bj = 90;
		trackBJ = 0;
		curX = 91;
		curY = 95;
		trackProtagonist = 1;
		hare_se_ve = 1;
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		loadPic("97g.alg", extraSurface);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		if (animate("lev.bin", 15))
			break;

		gotoObject(100 + curWidth / 2, 99 + curHeight);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		trackProtagonist = 1;
		curX = 100;
		curY = 95;

		talk_bj(2);
		talk(215);
		talk_bj(3);
		talk(216);
		talk_bj(4);
		talk_bj(5);
		talk_bj(6);
		talk(217);
		talk_bj(7);
		talk(218);
		talk_bj(8);
		talk(219);
		talk_bj(9);
		talk(220);
		talk(221);
		talk_bj(10);
		talk(222);
		if (animate("gaf.bin", 15))
			break;
		if (animate("bjb.bin", 14))
			break;
		playMusic(9);
		loadPic(97, extraSurface);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		updateScreen();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(120);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_text[_lang][223], "223.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = kColorWhite;
		updateRoom();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateScreen();
		pause(110);
		talk_solo(_textbj[_lang][11], "BJ11.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateScreen();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(118);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		gotoObject(132, 97 + curHeight);
		pause(60);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk(224);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_bj(12);
		gotoObject(157, 98 + curHeight);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (animate("bes.bin", 16))
			break;
		playMusic(11);
		if (animate("rap.bin", 16))
			break;
		trackProtagonist = 3;
		// The room number was originally changed here to "no_bj.alg",
		// which doesn't exist. In reality, this was just a hack to
		// set the room number to a non-existant one, so that BJ does
		// not appear again when the room is refreshed after the
		// animation where Count Drascula abducts her. We set the
		// room number to -1 for the same purpose
		// Also check animation_9_6(), where the same hack was used by
		// the original
		roomNumber = -1;
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(8);
		updateRoom();
		updateScreen();
		talk(225);
		pause(76);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		trackProtagonist = 1;
		updateRoom();
		updateScreen();
		talk(226);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		updateScreen();
		pause(30);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk(227);
		fadeToBlack(0);
		break;
	}
}

void DrasculaEngine::animation_3_1() {
	if (_lang == kSpanish)
		textSurface = frontSurface;

	loadPic("an11y13.alg", extraSurface);

	talk(192);
	talk_bartender(1);
	talk(193);
	talk_bartender(2);
	talk(194);
	talk_bartender(3);
	talk(195);
	talk_bartender(4);
	talk(196);
	talk_bartender(5);
	talk_bartender(6);
	talk(197);
	talk_bartender(7);
	talk(198);
	talk_bartender(8);
	talk(199);
	talk_bartender(9);
	talk(200);
	talk(201);
	talk(202);

	flags[0] = 1;

	if (_lang == kSpanish)
		textSurface = extraSurface;

	loadPic(97, extraSurface);
}

void DrasculaEngine::animation_4_1() {
	if (_lang == kSpanish)
		textSurface = frontSurface;

	loadPic("an12.alg", extraSurface);

	talk(205);

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	updateScreen(228,112, 228,112, 47,60, screenSurface);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	moveCharacters();

	updateScreen();

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianist(1);
	talk(206);
	talk_pianist(2);
	talk(207);
	talk_pianist(3);
	talk(208);
	talk_pianist(4);
	talk(209);

	if (_lang == kSpanish)
		textSurface = extraSurface;

	flags[11] = 0;
	loadPic(97, extraSurface);
}

void DrasculaEngine::animation_1_2() {
	gotoObject(178, 121);
	gotoObject(169, 135);
}

void DrasculaEngine::animation_2_2() {
	trackProtagonist = 0;
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	moveCharacters();
	updateRefresh();
	updateScreen();
	loadPic("an2_1.alg", frontSurface);
	loadPic("an2_2.alg", extraSurface);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	copyBackground(1, 1, 201, 87, 50, 52, frontSurface, screenSurface);
	updateScreen();

	updateAnim(1, 201, 87, 50, 52, 6, frontSurface);
	updateAnim(55, 201, 87, 50, 52, 6, frontSurface);
	updateAnim(109, 201, 87, 50, 52, 6, frontSurface);

	playSound(2);

	updateAnim(1, 201, 87, 50, 52, 6, extraSurface);
	updateAnim(55, 201, 87, 50, 52, 6, extraSurface);
	updateAnim(109, 201, 87, 50, 52, 2, extraSurface);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	finishSound();

	pause (4);

	playSound(1);
	hipo_sin_nadie(12);
	finishSound();
}

void DrasculaEngine::animation_3_2() {
	gotoObject(163, 106);
	gotoObject(287, 101);
	trackProtagonist = 0;
}

void DrasculaEngine::animation_4_2() {
	stopMusic();
	flags[9] = 1;

	pause(12);
	talk(56);
	pause(8);

	clearRoom();
	loadPic("ciego1.alg", drawSurface1, HALF_PAL);	// ciego = blind
	loadPic("ciego2.alg", drawSurface3);
	loadPic("ciego3.alg", extraSurface);
	loadPic("ciego4.alg", backSurface);
	loadPic("ciego5.alg", frontSurface);

	if (_lang == kSpanish)
		textSurface = frontSurface;

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	pause(10);

	talk_blind(1);
	pause(5);
	talk_hacker(_textd[_lang][57], "d57.als");
	pause(6);
	talk_blind(2);
	pause(4);
	talk_hacker(_textd[_lang][58],"d58.als");
	talk_blind(3);
	delay(14);
	talk_hacker(_textd[_lang][59],"d59.als");
	talk_blind(4);
	talk_hacker(_textd[_lang][60],"d60.als");
	talk_blind(5);
	talk_hacker(_textd[_lang][61],"d61.als");
	talk_blind(6);
	talk_hacker(_textd[_lang][62],"d62.als");
	talk_blind(7);
	talk_hacker(_textd[_lang][63],"d63.als");
	talk_blind(8);
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();
	_system->delayMillis(1000);
	talk_hacker(_textd[_lang][64], "d64.als");
	talk_blind(9);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	pause(14);

	clearRoom();

	playMusic(roomMusic);
	loadPic(9, drawSurface1, HALF_PAL);
	loadPic("aux9.alg", drawSurface3);
	loadPic(96, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);
	withoutVerb();

	if (_lang == kSpanish)
		textSurface = extraSurface;

	flags[9] = 0;
	flags[4] = 1;
}

void DrasculaEngine::animation_8_2() {
	talk_pianist(6);
	talk(358);
	talk_pianist(7);
	talk_pianist(8);
}

void DrasculaEngine::animation_9_2() {
	talk_pianist(9);
	talk_pianist(10);
	talk_pianist(11);
}

void DrasculaEngine::animation_10_2() {
	talk_pianist(12);
	talk(361);
	pause(40);
	talk_pianist(13);
	talk(362);
	talk_pianist(14);
	talk(363);
	talk_pianist(15);
	talk(364);
	talk_pianist(16);
}

void DrasculaEngine::animation_14_2() {
	int n, pos_cabina[6];
	int l = 0;

	loadPic("an14_2.alg", backSurface);

	pos_cabina[0] = 150;
	pos_cabina[1] = 6;
	pos_cabina[2] = 69;
	pos_cabina[3] = -160;
	pos_cabina[4] = 158;
	pos_cabina[5] = 161;

	for (n = -160; n <= 0; n = n + 5 + l) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		moveCharacters();
		moveVB();
		pos_cabina[3] = n;
		copyRectClip(pos_cabina, backSurface, screenSurface);
		updateRefresh();
		updateScreen();
		l = l + 1;
	}

	flags[24] = 1;

	memcpy(drawSurface1, screenSurface, 64000);

	playSound(7);
	hiccup(15);

	finishSound();

	loadPic(99, backSurface);
}

void DrasculaEngine::animation_15_2() {
	talk_drunk(8);
	pause(7);
	talk_drunk(9);
	talk_drunk(10);
	talk_drunk(11);
}

void DrasculaEngine::animation_16_2() {
	int l;

	talk_drunk(12);
	talk(371);

	clearRoom();

	if (_lang == kSpanish)
		playMusic(30);
	else
		playMusic(32);

	int key = getScan();
	if (key != 0)
		goto asco;

	if (_lang != kSpanish)
		color_abc(kColorDarkGreen);

	loadPic("his1.alg", drawSurface1, HALF_PAL);

	if (_lang == kSpanish)
		black();

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	if (_lang != kSpanish)
		centerText(_texthis[_lang][1], 180, 180);

	updateScreen();

	if (_lang == kSpanish)
		fadeFromBlack(1);

	key = getScan();
	if (key != 0)
		goto asco;

	if (_lang == kSpanish)
		_system->delayMillis(3000);
	else
		_system->delayMillis(4000);

	key = getScan();
	if (key != 0)
		goto asco;

	fadeToBlack(1);
	key = getScan();
	if (key != 0)
		goto asco;

	clearRoom();
	loadPic("his2.alg", drawSurface1, HALF_PAL);
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	if (_lang != kSpanish)
		centerText(_texthis[_lang][2], 180, 180);

	updateScreen();
	key = getScan();
	if (key != 0)
		goto asco;

	if (_lang == kSpanish)
		_system->delayMillis(3000);
	else
		_system->delayMillis(4000);

	key = getScan();
	if (key != 0)
		goto asco;

	fadeToBlack(1);
	key = getScan();
	if (key != 0)
		goto asco;

	clearRoom();
	loadPic("his3.alg", drawSurface1, HALF_PAL);
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	if (_lang != kSpanish)
		centerText(_texthis[_lang][3], 180, 180);

	updateScreen();
	key = getScan();
	if (key != 0)
		goto asco;

	if (_lang == kSpanish)
		_system->delayMillis(3000);
	else
		_system->delayMillis(4000);

	key = getScan();
	if (key != 0)
		goto asco;

	fadeToBlack(1);

	clearRoom();
	loadPic("his4_1.alg", drawSurface1, HALF_PAL);
	loadPic("his4_2.alg", drawSurface3);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface3, screenSurface);

	if (_lang != kSpanish)
		centerText(_texthis[_lang][1], 180, 180);

	updateScreen();
	key = getScan();
	if (key != 0)
		goto asco;

	if (_lang == kSpanish)
		_system->delayMillis(2000);
	else
		_system->delayMillis(4000);

	key = getScan();
	if (key != 0)
		goto asco;

	for (l = 1; l < 200; l++) {
		copyBackground(0, 0, 0, l, 320, 200 - l, drawSurface3, screenSurface);
		copyBackground(0, 200 - l, 0, 0, 320, l, drawSurface1, screenSurface);
		updateScreen();
		key = getScan();
		if (key != 0)
			goto asco;
	}

	pause(5);
	fadeToBlack(2);
	clearRoom();

asco:
	loadPic(roomDisk, drawSurface3);
	loadPic(roomNumber, drawSurface1, HALF_PAL);
	black();
	updateRoom();
	updateScreen();
	fadeFromBlack(0);
	if (roomMusic != 0)
		playMusic(roomMusic);
	else
		stopMusic();
}

void DrasculaEngine::animation_17_2() {
	talk_drunk(13);
	talk_drunk(14);
	flags[40] = 1;
}

void DrasculaEngine::animation_19_2() {
	talk_vbpuerta(5);
}

void DrasculaEngine::animation_20_2() {
	talk_vbpuerta(7);
	talk_vbpuerta(8);
	talk(383);
	talk_vbpuerta(9);
	talk(384);
	talk_vbpuerta(10);
	talk(385);
	talk_vbpuerta(11);
	if (flags[23] == 0) {
		talk(350);
		talk_vbpuerta(57);
	} else {
		talk(386);
		talk_vbpuerta(12);
		flags[18] = 0;
		flags[14] = 1;
		openDoor(15, 1);
		exitRoom(1);
		animation_23_2();
		exitRoom(0);
		flags[21] = 0;
		flags[24] = 0;
		trackVB = 1;
		vbX = 120;

		breakOut = 1;
	}
}

void DrasculaEngine::animation_21_2() {
	talk_vbpuerta(6);
}

void DrasculaEngine::animation_23_2() {
	loadPic("an24.alg", frontSurface);

	flags[21] = 1;

	if (flags[25] == 0) {
		talk_vb(13);
		talk_vb(14);
		pause(10);
		talk(387);
	}

	talk_vb(15);
	placeVB(42);
	trackVB = 1;
	talk_vb(16);
	trackVB = 2;
	gotoObject(157, 147);
	gotoObject(131, 149);
	trackProtagonist = 0;
	animation_14_2();
	if (flags[25] == 0)
		talk_vb(17);
	pause(8);
	trackVB = 1;
	talk_vb(18);

	if (flags[29] == 0)
		animation_23_joined();
	else
		animation_23_joined2();

	trackVB = 2;
	animation_25_2();
	placeVB(99);

	if (flags[29] == 0) {
		talk_vb(19);
		if (flags[25] == 0) {
			talk_vb(20);
			if (removeObject(7) == 0)
				flags[30] = 1;
			if (removeObject(18) == 0)
				flags[31] = 1;
			if (removeObject(19) == 0)
				flags[32] = 1;
		}
		talk_vb(21);
	} else
		animation_27_2();

	flags[25] = 1;
	breakOut = 1;
}

void DrasculaEngine::animation_23_joined() {
	int n, p_x = curX + 2, p_y = curY - 3;
	int x[] = {1, 38, 75, 112, 75, 112, 75, 112, 149, 112, 149, 112, 149, 186, 223, 260,
				1, 38, 75, 112, 149, 112, 149, 112, 149, 112, 149, 186, 223, 260, 260, 260, 260, 223};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 76, 76, 76, 76, 76, 76, 76,
				76, 76, 76, 76, 76, 76, 76, 1, 1, 1, 1};

	loadPic("an23.alg", backSurface);

	for (n = 0; n < 34; n++) {
		copyRect(p_x, p_y, p_x, p_y, 36, 74, drawSurface1, screenSurface);
		copyRect(x[n], y[n], p_x, p_y, 36, 74, backSurface, screenSurface);
		updateRefresh();
		updateScreen(p_x, p_y, p_x, p_y, 36, 74, screenSurface);
		pause(5);
	}

	loadPic(99, backSurface);
}

void DrasculaEngine::animation_23_joined2() {
	int n, p_x = curX + 4, p_y = curY;
	int x[] = {1, 35, 69, 103, 137, 171, 205, 239, 273, 1, 35, 69, 103, 137};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 73, 73, 73, 73, 73};

	pause(50);

	loadPic("an23_2.alg", backSurface);

	for (n = 0; n < 14; n++) {
		copyRect(p_x, p_y, p_x, p_y, 33, 71, drawSurface1, screenSurface);
		copyRect(x[n], y[n], p_x, p_y, 33, 71, backSurface, screenSurface);
		updateRefresh();
		updateScreen(p_x,p_y, p_x,p_y, 33,71, screenSurface);
		pause(5);
	}

	loadPic(99, backSurface);
}

void DrasculaEngine::animation_25_2() {
	int n, pos_cabina[6];

	loadPic("an14_2.alg", backSurface);
	loadPic(18, drawSurface1);

	pos_cabina[0] = 150;
	pos_cabina[1] = 6;
	pos_cabina[2] = 69;
	pos_cabina[3] = 0;
	pos_cabina[4] = 158;
	pos_cabina[5] = 161;

	flags[24] = 0;

	playSound(6);

	for (n = 0; n >= -160; n = n - 8) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();
		moveCharacters();
		moveVB();

		pos_cabina[3] = n;

		copyRectClip(pos_cabina, backSurface, screenSurface);

		updateRefresh();
		updateScreen();
	}

	finishSound();

	loadPic(99, backSurface);
}

void DrasculaEngine::animation_27_2() {
	flags[22] = 1;

	withoutVerb();
	removeObject(23);
	addObject(11);

	talk_vb(23);
	talk_vb(24);
	if (flags[30] == 1)
		addObject(7);
	if (flags[31] == 1)
		addObject(18);
	if (flags[32] == 1)
		addObject(19);
	talk_vb(25);
	talk_vb(26);
}

void DrasculaEngine::animation_28_2() {
	for(int i = 27; i <= 30; i++)
		talk_vb(i);
}

void DrasculaEngine::animation_29_2() {
	if (flags[33] == 0) {
		talk_vb(32);
		talk(398);
		talk_vb(33);
		talk(399);
		talk_vb(34);
		talk_vb(35);
		talk(400);
		talk_vb(36);
		talk_vb(37);
		talk(386);
		talk_vb(38);
		talk_vb(39);
		talk(401);
		talk_vb(40);
		talk_vb(41);
		flags[33] = 1;
	} else
		talk_vb(43);

	talk(402);
	talk_vb(42);

	if (flags[38] == 0) {
		talk(403);
		breakOut = 1;
	} else
		talk(386);
}

void DrasculaEngine::animation_30_2() {
	talk_vb(31);
	talk(396);
}

void DrasculaEngine::animation_31_2() {
	talk_vb(44);
	placeVB(-50);
	pause(15);
	gotoObject(159, 140);
	loadPic(99, backSurface);
	trackProtagonist = 2;
	updateRoom();
	updateScreen();
	pause(78);
	trackProtagonist = 0;
	updateRoom();
	updateScreen();
	pause(22);
	talk(406);
	placeVB(98);
	talk_vb(45);
	talk_vb(46);
	talk_vb(47);
	talk(407);
	talk_vb(48);
	talk_vb(49);
	talk(408);
	talk_vb(50);
	talk_vb(51);
	talk(409);
	talk_vb(52);
	talk_vb(53);
	pause(12);
	talk_vb(54);
	talk_vb(55);
	talk(410);
	talk_vb(56);

	breakOut = 1;

	flags[38] = 0;
	flags[36] = 1;
	withoutVerb();
	removeObject(8);
	removeObject(13);
	removeObject(15);
	removeObject(16);
	removeObject(17);
	addObject(20);
}

void DrasculaEngine::animation_35_2() {
	gotoObject(96, 165);
	gotoObject(79, 165);

	updateRoom();
	updateScreen();

	loadPic("an35_1.alg", backSurface);
	loadPic("an35_2.alg", frontSurface);

	updateAnim(1, 70, 90, 46, 80, 6, backSurface);
	updateAnim(82, 70, 90, 46, 80, 6, backSurface);
	updateAnim(1, 70, 90, 46, 80, 6, frontSurface);
	updateAnim(82, 70, 90, 46, 80, 2, frontSurface);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	updateScreen();

	pause(19);

	playSound(1);
	hipo_sin_nadie(18);
	finishSound();

	pause(10);

	fadeToBlack(2);
}

void DrasculaEngine::animation_1_3() {
	talk(413);
	grr();
	pause(50);
	talk(414);
}

void DrasculaEngine::animation_2_3() {
	flags[0] = 1;
	playMusic(13);
	animation_3_3();
	playMusic(13);
	animation_4_3();
	flags[1] = 1;
	updateRoom();
	updateScreen(120, 0, 120, 0, 200, 200, screenSurface);
	animation_5_3();
	flags[0] = 0;
	flags[1] = 1;

	loadPic(96, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);

	gotoObject(332, 127);
}

void DrasculaEngine::animation_3_3() {
	int px = curX - 20, py = curY - 1;

	loadPic("an2y_1.alg", frontSurface);
	loadPic("an2y_2.alg", extraSurface);
	loadPic("an2y_3.alg", backSurface);

	updateAnim2(2, px, py, 71, 72, 4, frontSurface);
	updateAnim2(75, px, py, 71, 72, 4, frontSurface);
	updateAnim2(2, px, py, 71, 72, 4, extraSurface);
	updateAnim2(75, px, py, 71, 72, 4, extraSurface);
	updateAnim2(2, px, py, 71, 72, 4, backSurface);
	updateAnim2(75, px, py, 71, 72, 4, backSurface);
}

void DrasculaEngine::animation_4_3() {
	int px = 120, py = 63;

	loadPic("any_1.alg", frontSurface);
	loadPic("any_2.alg", extraSurface);
	loadPic("any_3.alg", backSurface);

	updateAnim2(1, px, py, 77, 89, 4, frontSurface);
	updateAnim2(91, px, py, 77, 89, 4, frontSurface);
	updateAnim2(1, px, py, 77, 89, 4, extraSurface);
	updateAnim2(91, px, py, 77, 89, 4, extraSurface);
	updateAnim2(1, px, py, 77, 89, 4, backSurface);
	updateAnim2(91, px, py, 77, 89, 4, backSurface);
}

void DrasculaEngine::animation_5_3() {
	int px = curX - 20, py = curY - 1;

	loadPic("an3y_1.alg", frontSurface);
	loadPic("an3y_2.alg", extraSurface);
	loadPic("an3y_3.alg", backSurface);

	updateAnim2(2, px, py, 71, 72, 4, frontSurface);
	updateAnim2(75, px, py, 71, 72, 4, frontSurface);
	updateAnim2(2, px, py, 71, 72, 4, extraSurface);
	updateAnim2(75, px, py, 71, 72, 4, extraSurface);
	updateAnim2(2, px, py, 71, 72, 4, backSurface);
	updateAnim2(75, px, py, 71, 72, 4, backSurface);
}

void DrasculaEngine::animation_6_3() {
	int frame = 0, px = 112, py = 62;
	int yoda_x[] = { 3 ,82, 161, 240, 3, 82 };
	int yoda_y[] = { 3, 3, 3, 3, 94, 94 };

	characterMoved = 0;
	flags[3] = 1;
	updateRoom();
	updateScreen();

	flags[1] = 0;

	loadPic("an4y.alg", frontSurface);

	for (frame = 0; frame < 6; frame++) {
		pause(3);
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		copyRect(yoda_x[frame], yoda_y[frame], px, py,	78, 90,	frontSurface, screenSurface);
		updateScreen(px, py, px, py, 78, 90, screenSurface);
	}

	flags[2] = 1;

	loadPic(96, frontSurface);

	updateRoom();
	updateScreen();
}

void DrasculaEngine::animation_ray() {
	loadPic("anr_1.alg", frontSurface, HALF_PAL);
	loadPic("anr_2.alg", extraSurface);
	loadPic("anr_3.alg", backSurface);
	loadPic("anr_4.alg", drawSurface1);
	loadPic("anr_5.alg", drawSurface3);

	updateScreen(0, 0, 0, 0, 320, 200, frontSurface);

	pause(50);

	playSound(5);

	updateScreen(0, 0, 0, 0, 320, 200, extraSurface);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, backSurface);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, drawSurface1);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, backSurface);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, drawSurface3);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, frontSurface);
	finishSound();
}

void DrasculaEngine::animation_2_4() {
	talk_igor(16, kIgorSeated);
	talk(278);
	talk_igor(17, kIgorSeated);
	talk(279);
	talk_igor(18, kIgorSeated);
}

void DrasculaEngine::animation_3_4() {
	talk_igor(19, kIgorSeated);
	talk_igor(20, kIgorSeated);
	talk(281);
}

void DrasculaEngine::animation_4_4() {
	talk(287);
	talk_igor(21, kIgorSeated);
	talk(284);
	talk_igor(22, kIgorSeated);
	talk(285);
	talk_igor(23, kIgorSeated);
}

void DrasculaEngine::animation_7_4() {
	black();
	talk(427);
	fadeFromBlack(1);
	removeObject(8);
	removeObject(10);
	removeObject(12);
	removeObject(16);
	addObject(17);
	flags[30] = 0;
	flags[29] = 0;
}

void DrasculaEngine::animation_1_5() {
	if (flags[0] == 0) {
		talk(430);
		talk_bj(16);
		talk_bj(17);
		talk_bj(18);
		talk(217);
		talk_bj(19);
		talk(229);
		pause(5);
		gotoObject(114, 170);
		trackProtagonist = 3;
		talk(431);
		talk_bj(20);
		trackProtagonist = 2;
		pause(4);
		talk(438);
		roomX = 120;
		roomY = 157;
		walkToObject = 1;
		trackFinal = 1;
		startWalking();
		talk_bj(21);

		for (;;) {
			if (characterMoved == 0)
				break;
			updateRoom();
			updateScreen();
		}

		trackProtagonist = 1;
		talk(229);
		flags[0] = 1;
	}

	trackProtagonist = 1;
	converse(8);
}

void DrasculaEngine::animation_2_5() {
	talk_bj(22);
}

void DrasculaEngine::animation_3_5() {
	talk_bj(23);
	pickObject(10);
	breakOut = 1;
}

void DrasculaEngine::animation_4_5() {
	flags[7] = 1;
	updateRoom();
	updateScreen();
	talk(228);
	talk_wolf(1);
	talk_wolf(2);
	pause(23);
	talk(229);
	talk_wolf(3);
	talk_wolf(4);
	talk(230);
	talk_wolf(5);
	talk(231);
	talk_wolf(6);
	talk_wolf(7);
	pause(33);
	talk(232);
	talk_wolf(8);
}

void DrasculaEngine::animation_5_5(){
	int h;
	int frame = 0;
	int boneX[] = {1, 99, 197, 1, 99, 197, 1, 99, 197};
	int boneY[] = {1, 1, 1, 66, 66, 66, 131, 131, 131};
	int flyX[] = {1, 63, 125, 187, 249};
	int pixelX = curX - 53, pixelY = curY - 9;

	withoutVerb();
	removeObject(8);

	gotoObject(curX - 19, curY + curHeight);
	trackProtagonist = 1;
	updateRoom();
	updateScreen();

	loadPic("3an5_1.alg", backSurface);
	loadPic("3an5_2.alg", frontSurface);

	for (frame = 0; frame < 9; frame++) {
		pause(3);
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		copyRect(boneX[frame], boneY[frame], pixelX, pixelY, 97, 64, backSurface, screenSurface);
		updateScreen(pixelX, pixelY, pixelX,pixelY, 97,64, screenSurface);
	}

	copyBackground(52, 161, 198, 81, 26, 24, drawSurface3, screenSurface);
	updateScreen(198, 81, 198, 81, 26, 24, screenSurface);

	for (frame = 0; frame < 9; frame++) {
		pause(3);
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		copyRect(boneX[frame], boneY[frame], pixelX, pixelY, 97, 64, frontSurface, screenSurface);
		updateScreen(pixelX, pixelY, pixelX,pixelY, 97, 64, screenSurface);
	}

	flags[6] = 1;
	updateVisible();
	pause(12);

	loadPic(96, frontSurface);
	for (h = 0; h < (200 - 18); h++)
		copyBackground(0, 53, 0, h, 320, 19, frontSurface, screenSurface);

	updateScreen();

	loadPic(101, drawSurface1, HALF_PAL);
	loadPic("3an5_3.alg", backSurface);
	loadPic("3an5_4.alg", extraSurface);

	updateScreen(0, 0, 0, 0, 320, 200, drawSurface1);
	pause(9);
	for (frame = 0; frame < 5; frame++) {
		pause(3);
		copyBackground(flyX[frame], 1, 174, 79, 61, 109, backSurface, screenSurface);
		updateScreen(174, 79, 174, 79, 61, 109, screenSurface);
	}
	for (frame = 0; frame < 5; frame++) {
		pause(3);
		copyBackground(flyX[frame], 1, 174, 79, 61, 109, extraSurface, screenSurface);
		updateScreen(174, 79, 174, 79, 61, 109, screenSurface);
	}
	updateScreen(0, 0, 0, 0, 320, 200, drawSurface1);

	playSound(1);
	finishSound();

	loadPic(99, backSurface);
	loadPic(97, extraSurface);

	clearRoom();

	loadPic(49, drawSurface1, HALF_PAL);
}

void DrasculaEngine::animation_6_5() {
	talk_wolf(9);
	talk(234);
}

void DrasculaEngine::animation_7_5() {
	talk_wolf(10);
	talk(236);
	talk_wolf(11);
	talk_wolf(12);
	talk_wolf(13);
	pause(34);
	talk_wolf(14);
}

void DrasculaEngine::animation_8_5() {
	talk_wolf(15);
	talk(238);
	talk_wolf(16);
}

void DrasculaEngine::animation_9_5() {
	flags[4] = 1;
	talk(401);
	withoutVerb();
	removeObject(15);
}

void DrasculaEngine::animation_10_5() {
	flags[3] = 1;
	talk(401);
	withoutVerb();
	removeObject(12);
}

void DrasculaEngine::animation_11_5() {
	flags[9] = 1;
	if (flags[2] == 1 && flags[3] == 1 && flags[4] == 1)
		animation_12_5();
	else {
		flags[9] = 0;
		talk(33);
	}
}

void DrasculaEngine::animation_12_5() {
	DacPalette256 bgPalette1;
	DacPalette256 bgPalette2;
	DacPalette256 bgPalette3;

	int frame;
	const int rayX[] = {1, 46, 91, 136, 181, 226, 271, 181};
	const int frusky_x[] = {100, 139, 178, 217, 100, 178, 217, 139, 100, 139};
	const int elfrusky_x[] = {1, 68, 135, 1, 68, 135, 1, 68, 135, 68, 1, 135, 68, 135, 68};
	int color, component;
	char fade;

	playMusic(26);
	updateRoom();
	updateScreen();
	pause(27);
	animate("rayo1.bin", 23);
	playSound(5);
	animate("rayo2.bin", 17);
	trackProtagonist = 1;
	updateRoom();
	updateScreen();

	setDarkPalette();

	for (color = 0; color < 255; color++)
		for (component = 0; component < 3; component++) {
			bgPalette1[color][component] = gamePalette[color][component];
			bgPalette2[color][component] = gamePalette[color][component];
			bgPalette3[color][component] = gamePalette[color][component];
		}

	for (fade = 1; fade >= 0; fade--) {
		for (color = 0; color < 128; color++)
			for (component = 0; component < 3; component++)
				bgPalette1[color][component] = adjustToVGA(bgPalette1[color][component] - 8 + fade);
	}

	for (fade = 2; fade >= 0; fade--) {
		for (color = 0; color < 128; color++)
			for (component = 0; component < 3; component++)
				bgPalette2[color][component] = adjustToVGA(bgPalette2[color][component] - 8 + fade);
	}

	for (fade = 3; fade >= 0; fade--) {
		for (color = 0; color < 128; color++)
			for (component = 0; component < 3; component++)
				bgPalette3[color][component] = adjustToVGA(bgPalette3[color][component] - 8 + fade);
	}

	loadPic("3an11_1.alg", backSurface);

	for (frame = 0; frame < 8; frame++) {
		if (frame == 2 || frame == 4 || frame == 8 || frame==10)
			setPalette((byte *)&bgPalette1);
		else if (frame == 1 || frame == 5 || frame == 7 || frame == 9)
			setPalette((byte *)&bgPalette2);
		else
			setPalette((byte *)&bgPalette3);

		pause(4);
		updateRoom();
		copyRect(rayX[frame], 1, 41, 0, 44, 44, backSurface, screenSurface);
		copyRect(frusky_x[frame], 113, 205, 50, 38, 86, drawSurface3, screenSurface);
		updateScreen();
	}

	stopSound();

	for (frame = 0; frame < 15; frame++) {
		if (frame == 2 || frame == 4 || frame == 7 || frame == 9)
			setPalette((byte *)&bgPalette1);
		else if (frame == 1 || frame == 5)
			setPalette((byte *)&gamePalette);
		else
			setPalette((byte *)&bgPalette2);

		pause(4);
		updateRoom();
		copyRect(elfrusky_x[frame], 47, 192, 39, 66, 106, backSurface, screenSurface);
		updateScreen();
	}

	animate("frel.bin", 16);
	clearRoom();
	setBrightPalette();
	setPalette((byte *)&gamePalette);

	flags[1] = 1;

	animation_13_5();
	playSound(1);
	hiccup(12);
	finishSound();

	loadPic(99, backSurface);

	gotoObject(40, 169);
	gotoObject(-14, 175);

	doBreak = 1;
	previousMusic = roomMusic;
	hare_se_ve = 1;
	clearRoom();
	trackProtagonist = 1;
	characterMoved = 0;
	curX = -1;
	objExit = 104;
	withoutVerb();
	enterRoom(57);
}

void DrasculaEngine::animation_13_5() {
	int frank_x = 199;
	int frame = 0;
	int frus_x[] = {1, 46, 91, 136, 181, 226, 271};
	int frus_y[] = {1, 1, 1, 1, 1, 1, 1, 89};
	int pos_frusky[6];

	loadPic("auxfr.alg", backSurface);

	pos_frusky[3] = 81;
	pos_frusky[4] = 44;
	pos_frusky[5] = 87;
	pos_frusky[0] = 1;
	pos_frusky[1] = 1;
	pos_frusky[2] = frank_x;
	updateRoom();
	copyRectClip(pos_frusky, backSurface, screenSurface);
	updateScreen();
	pause(15);

	playMusic(18);

	for (;;) {
		updateRoom();
		pos_frusky[0] = frus_x[frame];
		pos_frusky[1] = frus_y[frame];
		pos_frusky[2] = frank_x;
		copyRectClip( pos_frusky, backSurface, screenSurface);
		updateScreen();
		frank_x = frank_x - 5;
		frame++;
		if (frank_x <= -45)
			break;
		if (frame == 7) {
			frame = 0;
			trackProtagonist = 3;
		}
		pause(6);
	}
}

void DrasculaEngine::animation_14_5() {
	flags[11] = 1;
	playSound(3);
	updateRoom();
	updateScreen(0, 0, 0,0 , 320, 200, screenSurface);
	finishSound();
	pause(17);
	trackProtagonist = 3;
	talk(246);
	gotoObject(89, 160);
	flags[10] = 1;
	playSound(7);
	updateRoom();
	updateScreen();
	finishSound();
	pause(14);
	trackProtagonist = 3;
	updateRoom();
	updateScreen();
	talk_solo(_textd[_lang][18], "d18.als");
	fadeToBlack(1);
}

void DrasculaEngine::animation_15_5() {
	talk_mus(4);
	talk_mus(5);
	talk_mus(6);
	talk(291);
	talk_mus(7);
}

void DrasculaEngine::animation_16_5() {
	talk_mus(8);
}

void DrasculaEngine::animation_17_5() {
	talk_mus(9);
}

void DrasculaEngine::animation_1_6() {
	int l;

	trackProtagonist = 0;
	curX = 103;
	curY = 108;
	flags[0] = 1;
	for (l = 0; l < 200; l++)
		factor_red[l] = 98;

	loadPic("auxig2.alg", frontSurface);
	loadPic("auxdr.alg", drawSurface2);
	loadPic("car.alg", backSurface);
	talk_drascula(19, 1);
	talk(247);
	talk_drascula(20, 1);
	talk_drascula(21, 1);
	talk(248);
	talk_drascula(22, 1);
	talk(249);
	talk_drascula(23, 1);
	converse(11);
	talk_drascula(26, 1);

	animate("fum.bin", 15);

	talk_drascula(27, 1);
	talk(254);
	talk_drascula(28, 1);
	talk(255);
	talk_drascula(29, 1);
	fadeToBlack(1);
	clearRoom();
	loadPic("time1.alg", screenSurface);
	updateScreen();
	delay(930);
	clearRoom();
	black();
	hare_se_ve = 0;
	flags[0] = 0;
	updateRoom();
	updateScreen();
	fadeFromBlack(1);
	talk(256);
	talk_drascula(30, 1);
	talk(257);
	fadeToBlack(0);
	clearRoom();
	loadPic("time1.alg", screenSurface);
	updateScreen();
	delay(900);
	clearRoom();
	black();
	updateRoom();
	updateScreen();
	fadeFromBlack(1);
	talk(258);
	talk_drascula(31, 1);
	animation_5_6();
	talk_drascula(32, 1);
	talk_igor(11, kIgorDch);
	trackIgor = 3;
	talk_drascula(33, 1);
	talk_igor(12, kIgorFront);
	talk_drascula(34, 1);
	trackDrascula = 0;
	talk_drascula(35);

	if (_lang == kSpanish)
		textSurface = extraSurface;

	clearRoom();
	enterRoom(102);
	activatePendulum();
}

void DrasculaEngine::animation_2_6() {
	talk_drascula(24, 1);
}

void DrasculaEngine::animation_3_6() {
	talk_drascula(24, 1);
}

void DrasculaEngine::animation_4_6() {
	talk_drascula(25, 1);
}

void DrasculaEngine::animation_5_6() {
	int n, pos_pen[6];

	pos_pen[0] = 1;
	pos_pen[1] = 29;
	pos_pen[2] = 204;
	pos_pen[3] = -125;
	pos_pen[4] = 18;
	pos_pen[5] = 125;

	animate("man.bin", 14);

	for (n = -125; n <= 0; n = n + 2) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		pos_pen[3] = n;
		copyRectClip(pos_pen, drawSurface3, screenSurface);

		updateRefresh();

		updateScreen();
		pause(2);
	}

	flags[3] = 1;
}

void DrasculaEngine::animation_6_6() {
	animate("rct.bin", 11);
	clearRoom();
	withoutVerb();
	removeObject(20);
	loadPic(96, frontSurface);
	loadPic(97, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);
	doBreak = 1;
	objExit = 104;
	curX = -1;
	withoutVerb();
	enterRoom(58);
	hare_se_ve = 1;
	trackProtagonist = 1;
	animate("hbp.bin", 14);

	trackProtagonist = 3;
	flags[0] = 1;
	flags[1] = 0;
	flags[2] = 1;
}

void DrasculaEngine::animation_7_6() {
	flags[8] = 1;
	updateVisible();
}

void DrasculaEngine::animation_9_6() {
	int v_cd;

	animate("fin.bin", 14);
	playMusic(13);
	flags[5] = 1;
	animate("drf.bin", 16);
	fadeToBlack(0);
	clearRoom();
	curX = -1;
	objExit = 108;
	enterRoom(59);
	// The room number was originally changed here to "nada.alg",
	// which is a non-existant file. In reality, this was just a
	// hack to set the room number to a non-existant one, so that
	// room sprites do not appear again when the room is refreshed.
	// We set the room number to -1 for the same purpose.
	// Also check animation_2_1(), where the same hack was used
	// by the original
	roomNumber = -1;
	loadPic("nota2.alg", drawSurface1, HALF_PAL);
	black();
	trackProtagonist = 1;
	curX -= 21;
	updateRoom();
	updateScreen();
	fadeFromBlack(0);
	pause(96);
	gotoObject(116, 178);
	trackProtagonist = 2;
	updateRoom();
	updateScreen();
	playMusic(9);
	clearRoom();
	loadPic("nota.alg", drawSurface1, COMPLETE_PAL);
	color_abc(kColorWhite);
	talk_solo(_textbj[_lang][24], "bj24.als");
	talk_solo(_textbj[_lang][25], "bj25.als");
	talk_solo(_textbj[_lang][26], "bj26.als");
	talk_solo(_textbj[_lang][27], "bj27.als");
	talk_solo(_textbj[_lang][28], "bj28.als");
	trackProtagonist = 3;
	clearRoom();
	loadPic(96, frontSurface, COMPLETE_PAL);
	loadPic("nota2.alg", drawSurface1, HALF_PAL);
	talk(296);
	talk(297);
	talk(298);
	trackProtagonist = 1;
	talk(299);
	talk(300);
	updateRoom();
	copyBackground(0, 0, 0, 0, 320, 200, screenSurface, drawSurface1);
	updateScreen();
	color_abc(kColorLightGreen);
	talk_solo("GOOOOOOOOOOOOOOOL", "s15.als");
	loadPic("nota2.alg", drawSurface1);
	trackProtagonist = 0;
	updateRoom();
	updateScreen();
	talk(301);
	v_cd = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16;
	v_cd = v_cd + 4;
	playMusic(17);
	fadeToBlack(1);
	clearRoom();
	playFLI("qpc.bin", 1);
	MusicFadeout();
	stopMusic();
	clearRoom();
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, v_cd * 16);
	playMusic(3);
	playFLI("crd.bin", 1);
	stopMusic();
}

void DrasculaEngine::animation_10_6() {
	playSound(14);
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateRefresh_pre();
	copyBackground(164, 85, 155, 48, 113, 114, drawSurface3, screenSurface);
	updateScreen();
	finishSound();
	talk_bartender(23, 1);
	flags[7] = 1;
}

void DrasculaEngine::animation_11_6() {
	talk_bartender(10, 1);
	talk(268);
	talk_bartender(11, 1);
}

void DrasculaEngine::animation_12_6() {
	talk_bartender(12, 1);
	talk(270);
	talk_bartender(13, 1);
	talk_bartender(14, 1);
}

void DrasculaEngine::animation_13_6() {
	talk_bartender(15, 1);
}

void DrasculaEngine::animation_14_6() {
	talk_bartender(24, 1);
	addObject(21);
	flags[10] = 1;
	breakOut = 1;
}

void DrasculaEngine::animation_15_6() {
	talk_bartender(16, 1);
}

void DrasculaEngine::animation_18_6() {
	flags[6] = 1;
	withoutVerb();
	removeObject(21);
	animate("beb.bin", 10);
}

void DrasculaEngine::animation_19_6() {
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	copyBackground(140, 23, 161, 69, 35, 80, drawSurface3, screenSurface);

	updateRefresh_pre();
	moveCharacters();
	updateScreen();
	pause(6);
	updateRoom();
	updateScreen();
	playSound(4);
	pause(6);
	finishSound();
}

void DrasculaEngine::animation_12_2() {
	if (_lang == kSpanish)
		textSurface = frontSurface;

	loadPic("an12.alg", extraSurface);

	talk(356);

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	updateScreen(228, 112, 228, 112, 47, 60, screenSurface);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	moveCharacters();

	updateScreen();

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianist(5);
	converse(1);

	if (_lang == kSpanish)
		textSurface = extraSurface;

	flags[11] = 0;
	loadPic(974, extraSurface);
}

void DrasculaEngine::animation_26_2() {
	if (_lang == kSpanish)
		textSurface = frontSurface;

	loadPic("an12.alg", extraSurface);

	talk(392);

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	updateScreen(228, 112, 228, 112, 47, 60, screenSurface);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	moveCharacters();

	updateScreen();

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianist(5);
	talk(393);
	talk_pianist(17);
	talk_pianist(18);
	talk_pianist(19);

	loadPic("an26.alg", extraSurface);

	updateAnim(1, 225, 113, 50, 59, 6, extraSurface);

	int	x = 0;
	for (int n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 61, 225, 113, 50, 59, extraSurface, screenSurface);
		updateScreen(225, 113, 225, 113, 50, 59, screenSurface);
		x = x + 50;
		if (n == 2)
			playSound(9);
		pause(3);
	}

	stopSound();

	updateAnim(121, 225, 113, 50, 59, 6, extraSurface);

	pickObject(11);
	removeObject(12);

	if (_lang == kSpanish)
		textSurface = extraSurface;

	flags[11] = 0;
	flags[39] = 1;
	loadPic(974, extraSurface);
	roomMusic = 16;
}

void DrasculaEngine::animation_11_2() {
	if (_lang == kSpanish)
		textSurface = frontSurface;

	loadPic("an11y13.alg", extraSurface);

	talk(352);
	talk_bartender(1);
	talk(353);
	talk_bartender(17);
	talk(354);
	talk_bartender(18);
	talk(355);
	pause(40);
	talk_bartender(82);

	if (_lang == kSpanish)
		textSurface = extraSurface;

	loadPic(974, extraSurface);
}

void DrasculaEngine::animation_13_2() {
	loadPic("an11y13.alg", frontSurface);

	if (flags[41] == 0) {
		talk(103);
		talk_drunk(4);
		flags[12] = 1;
		talk(367);
		talk_drunk(5);
		flags[12] = 1;
		talk(368);
		talk_drunk(6);
		talk_drunk(7);
		flags[41] = 1;
	}
	converse(2);

	loadPic(964, frontSurface);
}

void DrasculaEngine::animation_18_2() {
	talk(378);
	talk_vbpuerta(4);
	converse(3);
}

void DrasculaEngine::animation_22_2() {
	talk(374);

	trackProtagonist=2;
	updateRoom();
	updateScreen();
	playSound(13);
	finishSound();
	trackProtagonist = 1;

	talk_vbpuerta(1);
	talk(375);
	talk_vbpuerta(2);
	talk(376);
	talk_vbpuerta(3);

	flags[18] = 1;
}

void DrasculaEngine::animation_24_2() {
	if (curX < 178)
		gotoObject(208, 136);
	trackProtagonist = 3;
	updateRoom();
	pause(3);
	trackProtagonist = 0;

	talk(356);

	loadPic("an24.alg", frontSurface);

	animation_32_2();

	flags[21] = 1;

	talk_vb(22);

	if (flags[22] == 0)
		converse(4);
	else
		converse(5);

	exitRoom(0);
	flags[21] = 0;
	flags[24] = 0;
	trackVB = 1;
	vbX = 120;
}

void DrasculaEngine::animation_32_2() {
	loadPic("an32_1.alg", drawSurface3);
	loadPic("an32_2.alg", backSurface);

	updateAnim(1, 113, 53, 65, 81, 4, drawSurface3, 4);
	updateAnim(83, 113, 53, 65, 81, 4, drawSurface3, 4);
	updateAnim(1, 113, 53, 65, 81, 4, backSurface, 4);

	int x = 0;
	for (int n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 83, 113, 53, 65, 81, backSurface, screenSurface);
		updateScreen(113, 53, 113, 53, 65, 81, screenSurface);
		x = x + 65;
		if (n < 2)
			pause(4);
	}

	loadPic("aux18.alg", drawSurface3);
}

void DrasculaEngine::animation_34_2() {
	trackProtagonist = 1;
	updateRoom();
	updateScreen();

	loadPic("an34_1.alg", backSurface);
	loadPic("an34_2.alg", extraSurface);

	updateAnim(1, 218, 79, 83, 75, 3, backSurface);
	updateAnim(77, 218, 79, 83, 75, 3, backSurface);

	playSound(8);

	updateAnim(1, 218, 79, 83, 75, 3, extraSurface);

	finishSound();

	pause(30);

	copyBackground(1, 77, 218, 79, 83, 75, extraSurface, screenSurface);
	updateScreen(218, 79, 218, 79, 83, 75, screenSurface);
	pause(3);

	loadPic(994, backSurface);
	loadPic(974, extraSurface);
}

void DrasculaEngine::animation_36_2() {
	if (_lang == kSpanish)
		textSurface = frontSurface;

	loadPic("an11y13.alg", extraSurface);

	talk(404);
	talk_bartender(19);
	talk_bartender(20);
	talk_bartender(21);
	talk(355);
	pause(40);
	talk_bartender(82);

	if (_lang == kSpanish)
		textSurface = extraSurface;

	loadPic(974, extraSurface);
}

void DrasculaEngine::animation_7_2() {
	loadPic("an7_1.alg", backSurface);
	loadPic("an7_2.alg", extraSurface);
	loadPic("an7_3.alg", frontSurface);

	if (flags[3] == 1)
		copyBackground(258, 110, 85, 44, 23, 53, drawSurface3, drawSurface1);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	updateScreen();

	updateAnim2(1, 80, 64, 51, 73, 6, backSurface);
	updateAnim2(75, 80, 64, 51, 73, 6, backSurface);
	updateAnim2(1, 80, 64, 51, 73, 6, extraSurface);
	updateAnim2(75, 80, 64, 51, 73, 6, extraSurface);
	updateAnim2(1, 80, 64, 51, 73, 6, frontSurface);

	loadPic("an7_4.alg", backSurface);
	loadPic("an7_5.alg", extraSurface);
	loadPic("an7_6.alg", frontSurface);
	loadPic("an7_7.alg", drawSurface3);

	updateAnim2(1, 80, 64, 51, 73, 6, backSurface);
	updateAnim2(75, 80, 64, 51, 73, 6, backSurface);
	updateAnim2(1, 80, 64, 51, 73, 6, extraSurface);
	updateAnim2(75, 80, 64, 51, 73, 6, extraSurface);
	updateAnim2(1, 80, 64, 51, 73, 6, frontSurface);
	updateAnim2(75, 80, 64, 51, 73, 6, extraSurface);
	updateAnim2(1, 80, 64, 51, 73, 6, frontSurface);
	updateAnim2(75, 80, 64, 51, 73, 6, frontSurface);
	updateAnim2(1, 80, 64, 51, 73, 6, drawSurface3);
	updateAnim2(75, 80, 64, 51, 73, 2, drawSurface3);

	loadPic("an7_8.alg", backSurface);
	loadPic("an7_9.alg", extraSurface);

	updateAnim2(1, 80, 64, 51, 73, 6, backSurface);
	updateAnim2(75, 80, 64, 51, 73, 6, backSurface);
	updateAnim2(1, 80, 64, 51, 73, 6, extraSurface);

	copyBackground(80, 64, 80, 64, 51, 73, drawSurface1, screenSurface);
	copyRect(1, 75, 80, 64, 51, 73, extraSurface, screenSurface);
	updateScreen(80, 64, 80, 64, 51, 73, screenSurface);

	flags[37] = 1;

	if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
		flags[38] = 1;

	loadPic(99, backSurface);
	loadPic(97, extraSurface);
	loadPic(96, frontSurface);
	loadPic("aux3.alg", drawSurface3);
}

void DrasculaEngine::animation_5_2() {
	trackProtagonist = 0;
	updateRoom();
	updateScreen();

	loadPic("an5_1.alg", backSurface);
	loadPic("an5_2.alg", extraSurface);
	loadPic("an5_3.alg", frontSurface);
	loadPic("an5_4.alg", drawSurface3);

	copyBackground(1, 1, 213, 66,	53,84, backSurface, screenSurface);
	updateScreen();

	updateAnim(1, 213, 66, 52, 84, 6, backSurface);
	updateAnim(86, 213, 66, 52, 84, 6, backSurface);
	updateAnim(1, 213, 66, 52, 84, 6, extraSurface);
	updateAnim(1, 213, 66, 52, 84, 6, extraSurface);
	updateAnim(86, 213, 66, 52, 84, 6, extraSurface);
	updateAnim(1, 213, 66, 52, 84, 6, frontSurface);

	playSound(1);
	updateAnim(86, 213, 66, 52, 84, 6, frontSurface);
	stopSound();

	updateAnim(1, 213, 66, 52, 84, 6, drawSurface3);

	loadPic(994, backSurface);
	loadPic(974, extraSurface);
	loadPic(964, frontSurface);
	loadPic("aux5.alg", drawSurface3);
	flags[8] = 1;
	curX = curX - 4;
	talk_sync(_text[_lang][46], "46.als", "4442444244244");
	withoutVerb();
}

void DrasculaEngine::animation_6_2() {
	stopMusic();
	flags[9] = 1;

	if (_lang == kSpanish)
		textSurface = frontSurface;

	clearRoom();
	loadPic("ciego1.alg", drawSurface1, HALF_PAL);	// ciego = blind
	loadPic("ciego2.alg", drawSurface3);
	loadPic("ciego3.alg", extraSurface);
	loadPic("ciego4.alg", backSurface);
	loadPic("ciego5.alg", frontSurface);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	pause(1);

	if (flags[4] == 1)
		talk_hacker(_textd[_lang][66], "d66.als");
	pause(6);
	talk_blind(11);
	pause(4);
	talk_hacker(_textd[_lang][67], "d67.als");

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	pause(10);

	clearRoom();

	playMusic(roomMusic);
	loadPic(9, drawSurface1, HALF_PAL);
	loadPic("aux9.alg", drawSurface3);
	loadPic(96, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);
	withoutVerb();

	if (_lang == kSpanish)
		textSurface = extraSurface;

	flags[9] = 0;
}

void DrasculaEngine::animation_33_2() {
	stopMusic();
	flags[9] = 1;

	pause(12);
	talk(56);
	pause(8);

	clearRoom();
	loadPic("ciego1.alg", drawSurface1, HALF_PAL);	// ciego = blind
	loadPic("ciego2.alg", drawSurface3);
	loadPic("ciego3.alg", extraSurface);
	loadPic("ciego4.alg", backSurface);
	loadPic("ciego5.alg", frontSurface);

	if (_lang == kSpanish)
		textSurface = frontSurface;

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	pause(10);

	talk_blind(1);
	pause(5);
	talk_hacker(_textd[_lang][57], "d57.als");
	pause(6);
	_system->delayMillis(1000);
	talk_blind(10);
	talk_hacker(_textd[_lang][65], "d65.als");

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	pause(14);

	clearRoom();

	playMusic(roomMusic);
	loadPic(9, drawSurface1, HALF_PAL);
	loadPic("aux9.alg", drawSurface3);
	loadPic(96, frontSurface);
	loadPic(97, extraSurface);
	loadPic(99, backSurface);
	withoutVerb();

	if (_lang == kSpanish)
		textSurface = extraSurface;

	flags[33] = 1;
	flags[9] = 0;
}

void DrasculaEngine::animation_1_4() {
	if (flags[21] == 0) {
		strcpy(objName[2], "igor");
		talk(275);

		updateRefresh_pre();

		copyBackground(131, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
		updateScreen(199, 95, 199, 95, 50, 66, screenSurface);

		pause(3);

		updateRefresh_pre();

		copyBackground(182, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
		moveCharacters();

		updateScreen();

		pause(3);
		flags[18] = 1;
		flags[20] = 1;

		talk_igor(13, kIgorSeated);
		talk_igor(14, kIgorSeated);
		talk_igor(15, kIgorSeated);
		flags[21] = 1;
	} else {
		talk(356);

		updateRefresh_pre();

		copyBackground(131, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
		updateScreen(199, 95, 199, 95, 50, 66, screenSurface);
		pause(2);

		updateRefresh_pre();

		copyBackground(182, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
		moveCharacters();

		updateScreen();

		flags[18] = 1;
		flags[20] = 1;

		talk(276);
		pause(14);
		talk_igor(6, kIgorSeated);
	}

	converse(6);
	flags[20] = 0;
	flags[18] = 0;
}

void DrasculaEngine::animation_5_4(){
	trackProtagonist = 3;
	loadPic("anh_dr.alg", backSurface);
	gotoObject(99, 160);
	gotoObject(38, 177);
	hare_se_ve = 0;
	updateRoom();
	updateScreen();
	delay(800);
	animate("bio.bin", 14);
	flags[29] = 1;
	curX = 95;
	curY = 82;
	updateRoom();
	updateScreen();
	openDoor(2, 0);
	loadPic("auxigor.alg", frontSurface);
	igorX = 100;
	igorY = 65;
	talk_igor(29, kIgorFront);
	talk_igor(30, kIgorFront);
	loadPic(96, frontSurface);
	loadPic(99, backSurface);
	hare_se_ve = 1;
	fadeToBlack(0);
	exitRoom(0);
}

void DrasculaEngine::animation_6_4() {
	int prevRoom = roomNumber;

	roomNumber = 26;
	clearRoom();
	loadPic(26, drawSurface1, HALF_PAL);
	loadPic("aux26.alg", drawSurface3);
	loadPic("auxigor.alg", frontSurface);
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	update_26_pre();
	igorX = 104;
	igorY = 71;
	placeIgor();
	updateScreen();
	pause(40);
	talk_igor(26, kIgorFront);
	roomNumber = prevRoom;
	clearRoom();
	loadPic(96, frontSurface);
	loadPic(roomDisk, drawSurface3);
	loadPic(roomNumber, drawSurface1, HALF_PAL);
	withoutVerb();
	updateRoom();
}

void DrasculaEngine::animation_8_4() {
	int frame;
	int estanteria_x[] = {1, 75, 149, 223, 1, 75, 149, 223, 149, 223, 149, 223, 149, 223};
	int estanteria_y[] = {1, 1, 1, 1, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74};

	loadPic("an_8.alg", frontSurface);

	for (frame = 0; frame < 14; frame++) {
		pause(2);
		copyBackground(estanteria_x[frame], estanteria_y[frame], 77, 45, 73, 72, frontSurface, screenSurface);
		updateScreen(77, 45, 77, 45, 73, 72, screenSurface);
	}

	loadPic(96, frontSurface);
	openDoor(7, 2);
}

void DrasculaEngine::animation_9_4() {
	animate("st.bin", 14);
	fadeToBlack(1);
}


} // End of namespace Drascula
