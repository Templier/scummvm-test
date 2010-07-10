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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/debug-channels.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "engines/util.h"
 
#include "testbed/events.h"
#include "testbed/fs.h"
#include "testbed/graphics.h"
#include "testbed/misc.h"
#include "testbed/savegame.h"
#include "testbed/testbed.h"
 
namespace Testbed {
 
bool TestbedEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL) ? true : false;
}

TestbedEngine::TestbedEngine(OSystem *syst) 
 : Engine(syst) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().
 
	// Do not initialize graphics here
 
	// However this is the place to specify all default directories
 
	DebugMan.addDebugChannel(kTestbedLogOutput, "LOG", "Log of test results generated by testbed");
	DebugMan.addDebugChannel(kTestbedEngineDebug, "Debug", "Engine-specific debug statements");
	DebugMan.enableDebugChannel("LOG");

	// Initialize testsuites here
	// GFX
	Testsuite *ts = new GFXTestSuite();
	_testsuiteList.push_back(ts);
	// FS
	ts = new FSTestSuite();
	_testsuiteList.push_back(ts);
	// Savegames
	ts = new SaveGameTestSuite();
	_testsuiteList.push_back(ts);
	// Misc.
	ts = new MiscTestSuite();
	_testsuiteList.push_back(ts);
	// Events
	ts = new EventTestSuite();
	_testsuiteList.push_back(ts);
}
 
TestbedEngine::~TestbedEngine() {
	Testsuite::deleteWriteStream();
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
	
	for (Common::Array<Testsuite*>::const_iterator i = _testsuiteList.begin(); i != _testsuiteList.end(); ++i) {
		delete (*i);
	}
}

void TestbedEngine::enableTestsuite(const char *name, bool enable) {
	Common::Array<Testsuite*>::const_iterator iter;
	Common::String tsName(name);


	for (iter = _testsuiteList.begin(); iter != _testsuiteList.end(); iter++) {
		if (tsName.equalsIgnoreCase((*iter)->getName())) {
			(*iter)->enable(enable);
			break;
		}
	}
	
	return;
}

void TestbedEngine::invokeTestsuites() {
	Common::Array<Testsuite*>::const_iterator iter;
	
	for (iter = _testsuiteList.begin(); iter != _testsuiteList.end(); iter++) {
		if ((*iter)->isEnabled()) {
			(*iter)->execute();
			}
		}
}

Common::Error TestbedEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);
 
	// As of now we are using GUI::MessageDialog for interaction, Test if it works.
	// interactive mode could also be modified by a config parameter "non-interactive=1"
	// TODO: Implement that

	Common::String prompt("Welcome to the ScummVM testbed!\n"
						"It is a framework to test the various ScummVM subsystems namely GFX, Sound, FS, events etc.\n"
						"If you see this, it means interactive tests would run on this system :)");

	// To be set from config file
	// By default Interactive tests are enabled
	// XXX: disabling these as of now for fastly testing other tests
	// Testsuite::isSessionInteractive = false;

	if (Testsuite::isSessionInteractive) {
		Testsuite::logPrintf("Info! : Interactive tests are also being executed.\n");		
		Testsuite::displayMessage(prompt, "Proceed?");
	}
	
	// Enable the testsuites you want to execute
	enableTestsuite("FS", true);
	enableTestsuite("GFX", true);
	// enableTestsuite("savegames", true);
	// enableTestsuite("misc", true);
	// enableTestsuite("events", true);
	// invoke them
	invokeTestsuites();
	
	
	return Common::kNoError;
}
 
} // End of namespace Testbed
