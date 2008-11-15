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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
#include "dsoptions.h"
#include "dsmain.h"
#include "gui/dialog.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"
#include "gui/TabWidget.h"
#include "osystem_ds.h"
#include "engines/scumm/scumm.h"
#include "touchkeyboard.h"
#include "gui/PopUpWidget.h"

#define ALLOW_CPU_SCALER

#ifdef DS_SCUMM_BUILD
namespace Scumm {
	extern Common::StringList generateSavegameList(Scumm::ScummEngine *scumm, bool saveMode);
	extern Scumm::ScummEngine *g_scumm;
}
#endif

namespace DS {

DSOptionsDialog::DSOptionsDialog() : GUI::Dialog(0, 0, 320 - 10, 230 - 40) {

	new ButtonWidget(this, 10, 170, 72, 16, "Close", GUI::kCloseCmd, 'C');
	_tab = new GUI::TabWidget(this, 5, 5, 300, 230 - 20 - 40 - 10);

	_tab->addTab("Controls");

	_leftHandedCheckbox = new GUI::CheckboxWidget(_tab, 5, 5, 130, 20, "Left handed mode", 0, 'L');
	_indyFightCheckbox = new GUI::CheckboxWidget(_tab, 5, 20, 140, 20, "Indy fight controls", 0, 'I');
	_showCursorCheckbox = new GUI::CheckboxWidget(_tab, 150, 5, 130, 20, "Show mouse cursor", 0, 'T');
	_snapToBorderCheckbox = new GUI::CheckboxWidget(_tab, 150, 20, 130, 20, "Snap to edges", 0, 'T');

	new GUI::StaticTextWidget(_tab, 20, 35, 100, 15, "Touch X Offset", GUI::kTextAlignLeft);
	_touchX = new GUI::SliderWidget(_tab, 130, 35, 130, 12, 1);
	_touchX->setMinValue(-8);
	_touchX->setMaxValue(+8);
	_touchX->setValue(0);
	_touchX->setFlags(GUI::WIDGET_CLEARBG);

	new GUI::StaticTextWidget(_tab, 20, 50, 100, 15, "Touch Y Offset", GUI::kTextAlignLeft);
	_touchY = new GUI::SliderWidget(_tab, 130, 50, 130, 12, 2);
	_touchY->setMinValue(-8);
	_touchY->setMaxValue(+8);
	_touchY->setValue(0);
	_touchY->setFlags(GUI::WIDGET_CLEARBG);

	new GUI::StaticTextWidget(_tab, 130 + 65 - 10, 65, 20, 15, "0", GUI::kTextAlignCenter);
	new GUI::StaticTextWidget(_tab, 130 + 130 - 10, 65, 20, 15, "8", GUI::kTextAlignCenter);
	new GUI::StaticTextWidget(_tab, 130 - 20, 65, 20, 15, "-8", GUI::kTextAlignCenter);


	_touchPadStyle = new GUI::CheckboxWidget(_tab, 5, 80, 270, 20, "Use laptop trackpad-style cursor control", 0x20000001, 'T');
	_screenTaps = new GUI::CheckboxWidget(_tab, 5, 95, 285, 20, "Tap for left click, double tap right click", 0x20000002, 'T');

	_sensitivityLabel = new GUI::StaticTextWidget(_tab, 20, 110, 110, 15, "Sensitivity", GUI::kTextAlignLeft);
	_sensitivity = new GUI::SliderWidget(_tab, 130, 110, 130, 12, 1);
	_sensitivity->setMinValue(4);
	_sensitivity->setMaxValue(16);
	_sensitivity->setValue(8);
	_sensitivity->setFlags(GUI::WIDGET_CLEARBG);

	_tab->addTab("Graphics");

	new GUI::StaticTextWidget(_tab, 5, 70, 180, 15, "Initial top screen scale:", GUI::kTextAlignLeft);

	_100PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 85, 230, 20, "100%", 0x30000001, 'T');
	_150PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 100, 230, 20, "150%", 0x30000002, 'T');
	_200PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 115, 230, 20, "200%", 0x30000003, 'T');

	new GUI::StaticTextWidget(_tab, 5, 5, 180, 15, "Main screen scaling:", GUI::kTextAlignLeft);

	_hardScaler = new GUI::CheckboxWidget(_tab, 5, 20, 270, 20, "Hardware scale (fast, but low quality)", 0x10000001, 'T');
	_cpuScaler = new GUI::CheckboxWidget(_tab, 5, 35, 270, 20, "Software scale (good quality, but slower)", 0x10000002, 'S');
	_unscaledCheckbox = new GUI::CheckboxWidget(_tab, 5, 50, 270, 20, "Unscaled (you must scroll left and right)", 0x10000003, 'S');

	_tab->addTab("General");

	_highQualityAudioCheckbox = new GUI::CheckboxWidget(_tab, 5, 5, 250, 20, "High quality audio (slower) (reboot)", 0, 'T');
	_disablePowerOff = new GUI::CheckboxWidget(_tab, 5, 20, 200, 20, "Disable power off", 0, 'T');

	_tab->setActiveTab(0);

	_radioButtonMode = false;
	
#ifdef DS_SCUMM_BUILD
	if (!DS::isGBAMPAvailable()) {
//		addButton(this, 100, 140, "Delete Save", 'dels', 'D');
	}
#endif

//	new GUI::StaticTextWidget(this, 90, 10, 130, 15, "ScummVM DS Options", GUI::kTextAlignCenter);


//#ifdef ALLOW_CPU_SCALER
//	_cpuScaler = new GUI::CheckboxWidget(this, 160, 115, 90, 20, "CPU scaler", 0, 'T');
//#endif

	




#ifdef DS_SCUMM_BUILD
	_delDialog = new Scumm::SaveLoadChooser("Delete game:", "Delete", false, Scumm::g_scumm);
#endif

	if (ConfMan.hasKey("snaptoborder", "ds")) {
		_snapToBorderCheckbox->setState(ConfMan.getBool("snaptoborder", "ds"));
	} else {
#ifdef DS_BUILD_D
		_snapToBorderCheckbox->setState(true);
#else
		_snapToBorderCheckbox->setState(false);
#endif
	}

	if (ConfMan.hasKey("showcursor", "ds")) {
		_showCursorCheckbox->setState(ConfMan.getBool("showcursor", "ds"));
	} else {
		_showCursorCheckbox->setState(true);
	}

	if (ConfMan.hasKey("lefthanded", "ds")) {
		_leftHandedCheckbox->setState(ConfMan.getBool("lefthanded", "ds"));
	} else {
		_leftHandedCheckbox->setState(false);
	}

	if (ConfMan.hasKey("unscaled", "ds")) {
		_unscaledCheckbox->setState(ConfMan.getBool("unscaled", "ds"));
	} else {
		_unscaledCheckbox->setState(false);
	}

	
	if (ConfMan.hasKey("topscreenzoom", "ds")) {

		_100PercentCheckbox->setState(false);		
		_150PercentCheckbox->setState(false);		
		_200PercentCheckbox->setState(false);		
		
		switch (ConfMan.getInt("topscreenzoom", "ds"))
		{
			case 100: {
				_100PercentCheckbox->setState(true);
				break;
			}

			case 150: {
				_150PercentCheckbox->setState(true);
				break;
			}

			case 200: {
				_200PercentCheckbox->setState(true);
				break;
			}
		}

	} else if (ConfMan.hasKey("twohundredpercent", "ds")) {
		_200PercentCheckbox->setState(ConfMan.getBool("twohundredpercent", "ds"));
	} else {
		// No setting
		_150PercentCheckbox->setState(true);
	}

	if (ConfMan.hasKey("22khzaudio", "ds")) {
		_highQualityAudioCheckbox->setState(ConfMan.getBool("22khzaudio", "ds"));
	} else {
		_highQualityAudioCheckbox->setState(false);
	}

	if (ConfMan.hasKey("disablepoweroff", "ds")) {
		_disablePowerOff->setState(ConfMan.getBool("disablepoweroff", "ds"));
	} else {
		_disablePowerOff->setState(false);
	}

    #ifdef ALLOW_CPU_SCALER
	if (ConfMan.hasKey("cpu_scaler", "ds")) {
		_cpuScaler->setState(ConfMan.getBool("cpu_scaler", "ds"));
	} else {
		_cpuScaler->setState(false);
	}
    #endif

	_indyFightCheckbox->setState(DS::getIndyFightState());

	if (ConfMan.hasKey("xoffset", "ds")) {
		_touchX->setValue(ConfMan.getInt("xoffset", "ds"));
	} else {
		_touchX->setValue(0);
	}

	if (ConfMan.hasKey("yoffset", "ds")) {
		_touchY->setValue(ConfMan.getInt("yoffset", "ds"));
	} else {
		_touchY->setValue(0);
	}

	if (ConfMan.hasKey("sensitivity", "ds")) {
		_sensitivity->setValue(ConfMan.getInt("sensitivity", "ds"));
	} else {
		_sensitivity->setValue(8);
	}

	if (ConfMan.hasKey("touchpad", "ds")) {
		_touchPadStyle->setState(ConfMan.getBool("touchpad", "ds"));
	} else {
		_touchPadStyle->setState(0);
	}

	if (ConfMan.hasKey("screentaps", "ds")) {
		_screenTaps->setState(ConfMan.getBool("screentaps", "ds"));
	} else {
		_screenTaps->setState(0);
	}

	_screenTaps->setEnabled(!_touchPadStyle->getState());
	_sensitivity->setEnabled(_touchPadStyle->getState());
	_sensitivityLabel->setEnabled(_touchPadStyle->getState());
	_sensitivityLabel->draw();

	if (!_cpuScaler->getState() && !_unscaledCheckbox->getState()) {
		_hardScaler->setState(true);
	}
		
	_radioButtonMode = true;
}

DSOptionsDialog::~DSOptionsDialog() {
	DS::setIndyFightState(_indyFightCheckbox->getState());
	ConfMan.flushToDisk();
}

void DSOptionsDialog::updateConfigManager() {
	ConfMan.setBool("lefthanded", _leftHandedCheckbox->getState(), "ds");
	ConfMan.setBool("unscaled", _unscaledCheckbox->getState(), "ds");
//	ConfMan.setBool("twohundredpercent", _twoHundredPercentCheckbox->getState(), "ds");
	ConfMan.setBool("22khzaudio", _highQualityAudioCheckbox->getState(), "ds");
	ConfMan.setBool("disablepoweroff", _disablePowerOff->getState(), "ds");
#ifdef ALLOW_CPU_SCALER
	ConfMan.setBool("cpu_scaler", _cpuScaler->getState(), "ds");
#endif
	ConfMan.setInt("xoffset", _touchX->getValue(), "ds");
	ConfMan.setInt("yoffset", _touchY->getValue(), "ds");
	ConfMan.setBool("showcursor", _showCursorCheckbox->getState(), "ds");
	ConfMan.setBool("snaptoborder", _snapToBorderCheckbox->getState(), "ds");
	ConfMan.setBool("touchpad", _touchPadStyle->getState(), "ds");
	ConfMan.setBool("screentaps", _screenTaps->getState(), "ds");
	ConfMan.setInt("sensitivity", _sensitivity->getValue(), "ds");

	u32 zoomLevel = 150;

	if (_100PercentCheckbox->getState()) {
		zoomLevel = 100;
	} else if (_150PercentCheckbox->getState()) {
		zoomLevel = 150;
	} else if (_200PercentCheckbox->getState()) {
		zoomLevel = 200;
	}

	consolePrintf("Saved zoom: %d\n", zoomLevel);

	ConfMan.setInt("topscreenzoom", zoomLevel, "ds");

	DS::setOptions();
}

void DSOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	static bool guard = false;

	if ((!guard) && (_radioButtonMode))
	{
		guard = true;

		if ((cmd & 0xFF000000) == 0x10000000)
		{
			_cpuScaler->setState(false);
			_hardScaler->setState(false);
			_unscaledCheckbox->setState(false);
		
			if ((sender == _cpuScaler) && (cmd == 0x10000002))
			{
				_cpuScaler->setState(true);
			}
		
			if ((sender == _hardScaler) && (cmd == 0x10000001))
			{
				_hardScaler->setState(true);
			}
	
			if ((sender == _unscaledCheckbox) && (cmd == 0x10000003))
			{
				_unscaledCheckbox->setState(true);
			}
		}

		guard = false;

	}

	if ((!guard) && (_radioButtonMode))
	{
		guard = true;

		if ((sender == _touchPadStyle) && (cmd == 0x20000001)) {

			if (_touchPadStyle->getState()) {
				// Swap screens when turning on trackpad style, it feels
				// much more natural!
				DS::setGameScreenSwap(true);

				_screenTaps->setState(true);
				_screenTaps->setEnabled(false);
				_screenTaps->draw();
				_sensitivity->setEnabled(true);
				_sensitivityLabel->setEnabled(true);
				_sensitivityLabel->draw();
				_sensitivity->draw();
			} else {
				DS::setGameScreenSwap(false);

				_screenTaps->setEnabled(true);
				_screenTaps->setState(false);
				_screenTaps->draw();
				_sensitivity->setEnabled(false);
				_sensitivityLabel->setEnabled(false);
				_sensitivityLabel->draw();
				_sensitivity->draw();
			}
		}

		guard = false;
	}

	if ((!guard) && (_radioButtonMode)) {

		guard = true;

		if (cmd == 0x30000001) {
			_100PercentCheckbox->setState(true);
			_150PercentCheckbox->setState(false);
			_200PercentCheckbox->setState(false);
			DS::setTopScreenZoom(100);
		}

		if (cmd == 0x30000002) {
			_100PercentCheckbox->setState(false);
			_150PercentCheckbox->setState(true);
			_200PercentCheckbox->setState(false);
			DS::setTopScreenZoom(150);
		}

		if (cmd == 0x30000003) {
			_100PercentCheckbox->setState(false);
			_150PercentCheckbox->setState(false);
			_200PercentCheckbox->setState(true);
			DS::setTopScreenZoom(200);
		}

		guard = false;

	}


	if (cmd == GUI::kCloseCmd) {
		updateConfigManager();
		close();
	}
	
#ifdef DS_SCUMM_BUILD
/*	if (cmd == 'dels') {
		_delDialog->setList(Scumm::generateSavegameList(Scumm::g_scumm, false));
		_delDialog->handleCommand(NULL, GUI::kListSelectionChangedCmd, 0);
		
		Common::Event event;
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.ascii = 0;
		event.kbd.keycode = Common::KEYCODE_DOWN;
		OSystem_DS::instance()->addEvent(event);

		event.type = Common::EVENT_KEYUP;
		OSystem_DS::instance()->addEvent(event);
				
		int idx = _delDialog->runModal();
		
		if (idx >= 0) {
			char name[256];
			Scumm::g_scumm->makeSavegameName(name, idx, false);
			if (!DS::isGBAMPAvailable()) {
				((DSSaveFileManager *) (OSystem_DS::instance()->getSavefileManager()))->deleteFile(name);
			}
		}
		
	}*/
#endif
	

}

void togglePause() {
	// Toggle pause mode by simulating pressing 'p'.  Not a good way of doing things!

	if (getCurrentGame()->control == CONT_SCUMM_ORIGINAL) {
		Common::Event event;
		OSystem_DS* system = OSystem_DS::instance();

		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_p;		
		event.kbd.ascii = 'p';
		event.kbd.flags = 0;
		system->addEvent(event);
	
		event.type = Common::EVENT_KEYUP;
		system->addEvent(event);
	}
}

void showOptionsDialog() {

	togglePause();

	DS::displayMode16Bit();
	

	DSOptionsDialog* d = new DSOptionsDialog();
	d->runModal();
	delete d;
	
	DS::displayMode8Bit();

	togglePause();
}

void setOptions() {
	static bool firstLoad = true;

	ConfMan.addGameDomain("ds");

	if (ConfMan.hasKey("lefthanded", "ds")) {
		DS::setLeftHanded(ConfMan.getBool("lefthanded", "ds"));
	} else {
		DS::setLeftHanded(false);
	}

	if (ConfMan.hasKey("showcursor", "ds")) {
		DS::setMouseCursorVisible(ConfMan.getBool("showcursor", "ds"));
	} else {
		DS::setMouseCursorVisible(true);
	}

	if (ConfMan.hasKey("snaptoborder", "ds")) {
		DS::setSnapToBorder(ConfMan.getBool("snaptoborder", "ds"));
	} else {
#ifdef DS_BUILD_D
		DS::setSnapToBorder(true);
#else
		DS::setSnapToBorder(false);
#endif
	}

	if (ConfMan.hasKey("unscaled", "ds")) {
		DS::setUnscaledMode(ConfMan.getBool("unscaled", "ds"));
	} else {
		DS::setUnscaledMode(false);
	}

	if (firstLoad) {
		if (ConfMan.hasKey("topscreenzoom", "ds")) {
			DS::setTopScreenZoom(ConfMan.getInt("topscreenzoom", "ds"));
		} else {
			if (ConfMan.hasKey("twohundredpercent", "ds")) {
				DS::setTopScreenZoom(200);
			} else {
				DS::setTopScreenZoom(150);
			}
		}
	}

	if (ConfMan.hasKey("xoffset", "ds")) {
		DS::setTouchXOffset(ConfMan.getInt("xoffset", "ds"));
	} else {
		DS::setTouchXOffset(0);
	}

	if (ConfMan.hasKey("yoffset", "ds")) {
		DS::setTouchYOffset(ConfMan.getInt("yoffset", "ds"));
	} else {
		DS::setTouchXOffset(0);
	}

	if (ConfMan.hasKey("sensitivity", "ds")) {
		DS::setSensitivity(ConfMan.getInt("sensitivity", "ds"));
	} else {
		DS::setSensitivity(8);
	}

#ifdef ALLOW_CPU_SCALER
	if (ConfMan.hasKey("cpu_scaler", "ds")) {
		DS::setCpuScalerEnable(ConfMan.getBool("cpu_scaler", "ds"));
	} else {
		DS::setCpuScalerEnable(false);
	}
#endif	

	if (ConfMan.hasKey("screentaps", "ds")) {
		DS::setTapScreenClicksEnable(ConfMan.getBool("screentaps", "ds"));
	} else {
		DS::setTapScreenClicksEnable(false);
	}

	if (ConfMan.hasKey("touchpad", "ds")) {
		bool enable = ConfMan.getBool("touchpad", "ds");
		
		DS::setTrackPadStyleEnable(enable);

		if ((enable) and (firstLoad)) {
			// If we've just booted up, want to swap screens when trackpad mode is in use
			// but not every time we enter the options dialog.
			DS::setGameScreenSwap(true);
		}

		if (enable) {
			DS::setTapScreenClicksEnable(true);
		}
			
	} else {
		DS::setTrackPadStyleEnable(false);
	}


	firstLoad = false;
}

}

