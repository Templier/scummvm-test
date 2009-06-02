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

// Console module

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/resource.h"
#include "sci/vocabulary.h"
#include "sci/engine/savegame.h"
#include "sci/engine/state.h"
#include "sci/engine/gc.h"
#include "sci/engine/kernel_types.h"	// for determine_reg_type
#include "sci/gfx/gfx_gui.h"	// for sciw_set_status_bar
#include "sci/gfx/gfx_state_internal.h"
#include "sci/gfx/gfx_widgets.h"	// for getPort
#include "sci/sfx/songlib.h"	// for songlib_t
#include "sci/sfx/iterator.h"	// for SCI_SONG_ITERATOR_TYPE_SCI0
#include "sci/sfx/sci_midi.h"
#include "sci/vocabulary.h"

#include "common/savefile.h"

namespace Sci {

extern EngineState *g_EngineState;

int debug_sleeptime_factor = 1;
int _kdebug_cheap_event_hack = 0;
bool _kdebug_track_mouse_clicks = false;
int _weak_validations = 1; // Some validation errors are reduced to warnings if non-0


int parse_reg_t(EngineState *s, const char *str, reg_t *dest) { // Returns 0 on success
	int rel_offsetting = 0;
	const char *offsetting = NULL;
	// Non-NULL: Parse end of string for relative offsets
	char *endptr;

	if (*str == '$') { // Register
		rel_offsetting = 1;

		if (!scumm_strnicmp(str + 1, "PC", 2)) {
			*dest = s->_executionStack.back().addr.pc;
			offsetting = str + 3;
		} else if (!scumm_strnicmp(str + 1, "P", 1)) {
			*dest = s->_executionStack.back().addr.pc;
			offsetting = str + 2;
		} else if (!scumm_strnicmp(str + 1, "PREV", 4)) {
			*dest = s->r_prev;
			offsetting = str + 5;
		} else if (!scumm_strnicmp(str + 1, "ACC", 3)) {
			*dest = s->r_acc;
			offsetting = str + 4;
		} else if (!scumm_strnicmp(str + 1, "A", 1)) {
			*dest = s->r_acc;
			offsetting = str + 2;
		} else if (!scumm_strnicmp(str + 1, "OBJ", 3)) {
			*dest = s->_executionStack.back().objp;
			offsetting = str + 4;
		} else if (!scumm_strnicmp(str + 1, "O", 1)) {
			*dest = s->_executionStack.back().objp;
			offsetting = str + 2;
		} else
			return 1; // No matching register

		if (!*offsetting)
			offsetting = NULL;
		else if (*offsetting != '+' && *offsetting != '-')
			return 1;
	} else if (*str == '&') {
		int script_nr;
		// Look up by script ID
		char *colon = (char *)strchr(str, ':');

		if (!colon)
			return 1;
		*colon = 0;
		offsetting = colon + 1;

		script_nr = strtol(str + 1, &endptr, 10);

		if (*endptr)
			return 1;

		dest->segment = s->seg_manager->segGet(script_nr);

		if (!dest->segment) {
			return 1;
		}
	} else if (*str == '?') {
		int index = -1;
		int times_found = 0;
		char *tmp;
		const char *str_objname;
		char *str_suffix;
		char suffchar = 0;
		uint i;
		// Parse obj by name

		tmp = (char *)strchr(str, '+');
		str_suffix = (char *)strchr(str, '-');
		if (tmp < str_suffix)
			str_suffix = tmp;
		if (str_suffix) {
			suffchar = (*str_suffix);
			*str_suffix = 0;
		}

		tmp = (char *)strchr(str, '.');

		if (tmp) {
			*tmp = 0;
			index = strtol(tmp + 1, &endptr, 16);
			if (*endptr)
				return -1;
		}

		str_objname = str + 1;

		// Now all values are available; iterate over all objects.
		for (i = 0; i < s->seg_manager->_heap.size(); i++) {
			MemObject *mobj = s->seg_manager->_heap[i];
			int idx = 0;
			int max_index = 0;

			if (mobj) {
				if (mobj->getType() == MEM_OBJ_SCRIPT)
					max_index = (*(Script *)mobj)._objects.size();
				else if (mobj->getType() == MEM_OBJ_CLONES)
					max_index = (*(CloneTable *)mobj)._table.size();
			}

			while (idx < max_index) {
				int valid = 1;
				Object *obj = NULL;
				reg_t objpos;
				objpos.offset = 0;
				objpos.segment = i;

				if (mobj->getType() == MEM_OBJ_SCRIPT) {
					obj = &(*(Script *)mobj)._objects[idx];
					objpos.offset = obj->pos.offset;
				} else if (mobj->getType() == MEM_OBJ_CLONES) {
					obj = &((*(CloneTable *)mobj)._table[idx]);
					objpos.offset = idx;
					valid = ((CloneTable *)mobj)->isValidEntry(idx);
				}

				if (valid) {
					const char *objname = obj_get_name(s, objpos);
					if (!strcmp(objname, str_objname)) {
						// Found a match!
						if ((index < 0) && (times_found > 0)) {
							if (times_found == 1) {
								// First time we realized the ambiguity
								printf("Ambiguous:\n");
								printf("  %3x: [%04x:%04x] %s\n", 0, PRINT_REG(*dest), str_objname);
							}
							printf("  %3x: [%04x:%04x] %s\n", times_found, PRINT_REG(objpos), str_objname);
						}
						if (index < 0 || times_found == index)
							*dest = objpos;
						++times_found;
					}
				}
				++idx;
			}

		}

		if (!times_found)
			return 1;

		if (times_found > 1 && index < 0) {
			printf("Ambiguous: Aborting.\n");
			return 1; // Ambiguous
		}

		if (times_found <= index)
			return 1; // Not found

		offsetting = str_suffix;
		if (offsetting)
			*str_suffix = suffchar;
		rel_offsetting = 1;
	} else {
		char *colon = (char *)strchr(str, ':');

		if (!colon) {
			offsetting = str;
			dest->segment = 0;
		} else {
			*colon = 0;
			offsetting = colon + 1;

			dest->segment = strtol(str, &endptr, 16);
			if (*endptr)
				return 1;
		}
	}
	if (offsetting) {
		int val = strtol(offsetting, &endptr, 16);

		if (rel_offsetting)
			dest->offset += val;
		else
			dest->offset = val;

		if (*endptr)
			return 1;
	}

	return 0;
}

Console::Console(SciEngine *vm) : GUI::Debugger() {
	_vm = vm;

	// Kernel
//	DCmd_Register("classes",			WRAP_METHOD(Console, cmdClasses));	// TODO
	DCmd_Register("opcodes",			WRAP_METHOD(Console, cmdOpcodes));
	DCmd_Register("selectors",			WRAP_METHOD(Console, cmdSelectors));
	DCmd_Register("kernel_names",		WRAP_METHOD(Console, cmdKernelNames));
	DCmd_Register("registers",			WRAP_METHOD(Console, cmdRegisters));
	DCmd_Register("weak_validations",	WRAP_METHOD(Console, cmdWeakValidations));
	// Parser
	DCmd_Register("suffixes",			WRAP_METHOD(Console, cmdSuffixes));
	DCmd_Register("parse_grammar",		WRAP_METHOD(Console, cmdParseGrammar));
	DCmd_Register("parser_nodes",		WRAP_METHOD(Console, cmdParserNodes));
	DCmd_Register("parser_words",		WRAP_METHOD(Console, cmdParserWords));
	// Resources
	DCmd_Register("hexdump",			WRAP_METHOD(Console, cmdHexDump));
	DCmd_Register("resource_id",		WRAP_METHOD(Console, cmdResourceId));
	DCmd_Register("resource_size",		WRAP_METHOD(Console, cmdResourceSize));
	DCmd_Register("resource_types",		WRAP_METHOD(Console, cmdResourceTypes));
	DCmd_Register("list",				WRAP_METHOD(Console, cmdList));
	DCmd_Register("hexgrep",			WRAP_METHOD(Console, cmdHexgrep));
	// Game
	DCmd_Register("save_game",			WRAP_METHOD(Console, cmdSaveGame));
	DCmd_Register("restore_game",		WRAP_METHOD(Console, cmdRestoreGame));
	DCmd_Register("restart_game",		WRAP_METHOD(Console, cmdRestartGame));
	DCmd_Register("version",			WRAP_METHOD(Console, cmdGetVersion));
	DCmd_Register("room",				WRAP_METHOD(Console, cmdRoomNumber));
	// Graphics
	DCmd_Register("sci0_palette",		WRAP_METHOD(Console, cmdSci0Palette));
	DCmd_Register("clear_screen",		WRAP_METHOD(Console, cmdClearScreen));
	DCmd_Register("redraw_screen",		WRAP_METHOD(Console, cmdRedrawScreen));
	DCmd_Register("class_table",		WRAP_METHOD(Console, cmdClassTable));
	DCmd_Register("sentence_fragments",	WRAP_METHOD(Console, cmdSentenceFragments));
	DCmd_Register("draw_pic",			WRAP_METHOD(Console, cmdDrawPic));
	DCmd_Register("draw_rect",			WRAP_METHOD(Console, cmdDrawRect));
	DCmd_Register("draw_cel",			WRAP_METHOD(Console, cmdDrawCel));
	DCmd_Register("view_info",			WRAP_METHOD(Console, cmdViewInfo));
	DCmd_Register("update_zone",		WRAP_METHOD(Console, cmdUpdateZone));
	DCmd_Register("propagate_zone",		WRAP_METHOD(Console, cmdPropagateZone));
	DCmd_Register("fill_screen",		WRAP_METHOD(Console, cmdFillScreen));
	DCmd_Register("current_port",		WRAP_METHOD(Console, cmdCurrentPort));
	DCmd_Register("print_port",			WRAP_METHOD(Console, cmdPrintPort));
	DCmd_Register("visual_state",		WRAP_METHOD(Console, cmdVisualState));
	DCmd_Register("flush_visual",		WRAP_METHOD(Console, cmdFlushPorts));
	DCmd_Register("dynamic_views",		WRAP_METHOD(Console, cmdDynamicViews));
	DCmd_Register("dropped_views",		WRAP_METHOD(Console, cmdDroppedViews));
	DCmd_Register("priority_bands",		WRAP_METHOD(Console, cmdPriorityBands));
	DCmd_Register("status_bar",			WRAP_METHOD(Console, cmdStatusBarColors));
	DCmd_Register("show_map",			WRAP_METHOD(Console, cmdShowMap));
	// Events
	DCmd_Register("simkey",				WRAP_METHOD(Console, cmdSimulateKey));
	DCmd_Register("track_mouse",		WRAP_METHOD(Console, cmdTrackMouse));
	// Segments
	DCmd_Register("segment_table",		WRAP_METHOD(Console, cmdPrintSegmentTable));
	DCmd_Register("segment_info",		WRAP_METHOD(Console, cmdSegmentInfo));
	DCmd_Register("segment_kill",		WRAP_METHOD(Console, cmdKillSegment));
	// Garbage collection
	DCmd_Register("gc",					WRAP_METHOD(Console, cmdGCInvoke));
	DCmd_Register("gc_objects",			WRAP_METHOD(Console, cmdGCObjects));
	DCmd_Register("gc_interval",		WRAP_METHOD(Console, cmdGCInterval));
	DCmd_Register("gc_reachable",		WRAP_METHOD(Console, cmdGCShowReachable));
	DCmd_Register("gc_freeable",		WRAP_METHOD(Console, cmdGCShowFreeable));
	DCmd_Register("gc_normalize",		WRAP_METHOD(Console, cmdGCNormalize));
	// Music/SFX
	DCmd_Register("songlib",			WRAP_METHOD(Console, cmdSongLib));
	DCmd_Register("is_sample",			WRAP_METHOD(Console, cmdIsSample));
	DCmd_Register("sfx01_header",		WRAP_METHOD(Console, cmdSfx01Header));
	DCmd_Register("sfx01_track",		WRAP_METHOD(Console, cmdSfx01Track));
	DCmd_Register("stop_sfx",			WRAP_METHOD(Console, cmdStopSfx));
	// Script
	DCmd_Register("addresses",			WRAP_METHOD(Console, cmdAddresses));
	DCmd_Register("dissect_script",		WRAP_METHOD(Console, cmdDissectScript));
	DCmd_Register("script_steps",		WRAP_METHOD(Console, cmdScriptSteps));
	DCmd_Register("set_acc",			WRAP_METHOD(Console, cmdSetAccumulator));
	// VM
	DCmd_Register("vm_varlist",			WRAP_METHOD(Console, cmdVMVarlist));
	DCmd_Register("stack",				WRAP_METHOD(Console, cmdStack));
	DCmd_Register("value_type",			WRAP_METHOD(Console, cmdValueType));
	DCmd_Register("sleep_factor",		WRAP_METHOD(Console, cmdSleepFactor));
	DCmd_Register("exit",				WRAP_METHOD(Console, cmdExit));

	// These were in sci.cpp
	/*
	con_hook_int(&(gfx_options.buffer_pics_nr), "buffer_pics_nr",
		"Number of pics to buffer in LRU storage\n");
	con_hook_int(&(gfx_options.pic0_dither_mode), "pic0_dither_mode",
		"Mode to use for pic0 dithering\n");
	con_hook_int(&(gfx_options.pic0_dither_pattern), "pic0_dither_pattern",
		"Pattern to use for pic0 dithering\n");
	con_hook_int(&(gfx_options.pic0_unscaled), "pic0_unscaled",
		"Whether pic0 should be drawn unscaled\n");
	con_hook_int(&(gfx_options.dirty_frames), "dirty_frames",
		"Dirty frames management\n");
	*/
}

Console::~Console() {
}

static ResourceType parseResourceType(const char *resid) {
	// Gets the resource number of a resource string, or returns -1
	ResourceType res = kResourceTypeInvalid;

	for (int i = 0; i < kResourceTypeInvalid; i++)
		if (strcmp(getResourceTypeName((ResourceType)i), resid) == 0)
			res = (ResourceType)i;

	return res;
}

bool Console::cmdGetVersion(int argc, const char **argv) {
	int ver = _vm->getVersion();

	DebugPrintf("Resource file version:        %s\n", sci_version_types[_vm->getResMgr()->_sciVersion]);
	DebugPrintf("Emulated interpreter version: %s\n", versionNames[ver]);

	return true;
}

bool Console::cmdOpcodes(int argc, const char **argv) {
	DebugPrintf("Opcode names in numeric order [index: type name]:\n");
	for (uint seeker = 0; seeker < g_EngineState->_kernel->getOpcodesSize(); seeker++) {
		opcode op = g_EngineState->_kernel->getOpcode(seeker);
		DebugPrintf("%03x: %03x %20s | ", seeker, op.type, op.name.c_str());
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdSelectors(int argc, const char **argv) {
	DebugPrintf("Selector names in numeric order:\n");
	for (uint seeker = 0; seeker < g_EngineState->_kernel->getSelectorNamesSize(); seeker++) {
		DebugPrintf("%03x: %20s | ", seeker, g_EngineState->_kernel->getSelectorName(seeker).c_str());
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdKernelNames(int argc, const char **argv) {
	DebugPrintf("Selector names in numeric order:\n");
	for (uint seeker = 0; seeker <  g_EngineState->_kernel->getKernelNamesSize(); seeker++) {
		DebugPrintf("%03x: %20s | ", seeker, g_EngineState->_kernel->getKernelName(seeker).c_str());
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdSuffixes(int argc, const char **argv) {
	g_EngineState->_vocabulary->printSuffixes();

	return true;
}

bool Console::cmdParserWords(int argc, const char **argv) {
	g_EngineState->_vocabulary->printParserWords();

	return true;
}

bool Console::cmdRegisters(int argc, const char **argv) {
	DebugPrintf("Current register values:\n");
#if 0
		// TODO: p_restadjust
	DebugPrintf("acc=%04x:%04x prev=%04x:%04x &rest=%x\n", PRINT_REG(g_EngineState->r_acc), PRINT_REG(g_EngineState->r_prev), *p_restadjust);
#endif

	if (!g_EngineState->_executionStack.empty()) {
#if 0
		// TODO: p_pc, p_objp, p_pp, p_sp
		DebugPrintf("pc=%04x:%04x obj=%04x:%04x fp=ST:%04x sp=ST:%04x\n", PRINT_REG(*p_pc), PRINT_REG(*p_objp), PRINT_STK(*p_pp), PRINT_STK(*p_sp));
#endif
	} else
		DebugPrintf("<no execution stack: pc,obj,fp omitted>\n");

	return true;
}

bool Console::cmdHexDump(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		cmdResourceTypes(argc, argv);
		return true;
	}

	int resNum = atoi(argv[2]);
	if (resNum == 0) {
		DebugPrintf("The resource number specified is not a number");
		return true;
	}

	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _vm->getResMgr()->findResource(res, resNum, 0);
		if (resource) {
			Common::hexdump(resource->data, resource->size, 16, 0);
			DebugPrintf("Resource %s.%03d has been dumped to standard output\n", argv[1], resNum);
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceId(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Identifies a resource number by splitting it up in resource type and resource number\n");
		DebugPrintf("Usage: %s <resource number>\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	DebugPrintf("%s.%d (0x%x)\n", getResourceTypeName((ResourceType)(id >> 11)), id & 0x7ff, id & 0x7ff);

	return true;
}

bool Console::cmdDissectScript(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Examines a script\n");
		DebugPrintf("Usage: %s <script number>\n", argv[0]);
		return true;
	}

	g_EngineState->_kernel->dissectScript(atoi(argv[1]), g_EngineState->_vocabulary);

	return true;
}

bool Console::cmdWeakValidations(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Turns some validation errors into warnings\n");
		DebugPrintf("Usage: %s <on/off>\n", argv[0]);
		DebugPrintf("Weak validations are currently %s\n", (_weak_validations ? "on" : "off"));
		return true;
	}

	DebugPrintf("Weak validations are currently %s\n", (_weak_validations ? "on" : "off"));
	if (!scumm_stricmp(argv[1], "on")) {
		DebugPrintf("Turning weak validations on\n");
		_weak_validations = 1;
	} else if (!scumm_stricmp(argv[1], "off")) {
		DebugPrintf("Turning weak validations off\n");
		_weak_validations = 0;
	}

	return true;
}

bool Console::cmdRoomNumber(int argc, const char **argv) {
	DebugPrintf("Current room number is %d\n", g_EngineState->currentRoomNumber());

	return true;
}

bool Console::cmdResourceSize(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Shows the size of a resource\n");
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		return true;
	}

	int resNum = atoi(argv[2]);
	if (resNum == 0) {
		DebugPrintf("The resource number specified is not a number");
		return true;
	}

	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _vm->getResMgr()->findResource(res, resNum, 0);
		if (resource) {
			DebugPrintf("Resource size: %d\n", resource->size);
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceTypes(int argc, const char **argv) {
	DebugPrintf("The %d valid resource types are:\n", kResourceTypeInvalid);
	for (int i = 0; i < kResourceTypeInvalid; i++) {
		DebugPrintf("%s", getResourceTypeName((ResourceType) i));
		DebugPrintf((i < kResourceTypeInvalid - 1) ? ", " : "\n");
	}

	return true;
}

extern int sci0_palette;

bool Console::cmdSci0Palette(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Set the SCI0 palette to use - 0: EGA, 1: AGI/Amiga, 2: Grayscale\n");
		return true;
	}

	sci0_palette = atoi(argv[1]);
	cmdRedrawScreen(argc, argv);

	return false;
}

bool Console::cmdHexgrep(int argc, const char **argv) {
	if (argc < 4) {
		DebugPrintf("Searches some resources for a particular sequence of bytes, represented as hexadecimal numbers.\n");
		DebugPrintf("Usage: %s <resource type> <resource number> <search string>\n", argv[0]);
		DebugPrintf("<resource number> can be a specific resource number, or \"all\" for all of the resources of the specified type\n", argv[0]);
		DebugPrintf("EXAMPLES:\n  hexgrep script all e8 03 c8 00\n  hexgrep pic 042 fe");
		cmdResourceTypes(argc, argv);
		return true;
	}

	ResourceType restype = parseResourceType(argv[1]);
	int resNumber = 0, resMax = 0;
	char seekString[500];
	Resource *script = NULL;

	if (restype == kResourceTypeInvalid) {
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
		return true;
	}

	if (!scumm_stricmp(argv[2], "all")) {
		resNumber = 0;
		resMax = 999;
	} else {
		resNumber = resMax = atoi(argv[2]);
	}

	strcpy(seekString, argv[3]);

	// Construct the seek string
	for (int i = 4; i < argc; i++) {
		strcat(seekString, argv[i]);
	}

	for (; resNumber <= resMax; resNumber++) {
		if ((script = _vm->getResMgr()->findResource(restype, resNumber, 0))) {
			unsigned int seeker = 0, seekerold = 0;
			uint32 comppos = 0;
			int output_script_name = 0;

			while (seeker < script->size) {
				if (script->data[seeker] == seekString[comppos]) {
					if (comppos == 0)
						seekerold = seeker;

					comppos++;

					if (comppos == strlen(seekString)) {
						comppos = 0;
						seeker = seekerold + 1;

						if (!output_script_name) {
							DebugPrintf("\nIn %s.%03d:\n", getResourceTypeName((ResourceType)restype), resNumber);
							output_script_name = 1;
						}
						DebugPrintf("   0x%04x\n", seekerold);
					}
				} else
					comppos = 0;

				seeker++;
			}
		}
	}

	return true;
}

bool Console::cmdList(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Lists all of the resources of a given type\n");
		cmdResourceTypes(argc, argv);
		return true;
	}


	ResourceType res = parseResourceType(argv[1]);
	if (res == kResourceTypeInvalid)
		DebugPrintf("Unknown resource type: '%s'\n", argv[1]);
	else {
		int j = 0;
		for (int i = 0; i < sci_max_resource_nr[_vm->getResMgr()->_sciVersion]; i++) {
			if (_vm->getResMgr()->testResource(res, i)) {
				DebugPrintf("%s.%03d | ", getResourceTypeName((ResourceType)res), i);
				if (j % 5 == 0)
					DebugPrintf("\n");
				j++;
			}
		}
		DebugPrintf("\n");
	}

	return true;
}

bool Console::cmdClearScreen(int argc, const char **argv) {
	gfxop_clear_box(g_EngineState->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update_box(g_EngineState->gfx_state, gfx_rect(0, 0, 320, 200));
	return false;
}

bool Console::cmdRedrawScreen(int argc, const char **argv) {
	g_EngineState->visual->draw(Common::Point(0, 0));
	gfxop_update_box(g_EngineState->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update(g_EngineState->gfx_state);
	gfxop_sleep(g_EngineState->gfx_state, 0);
	return false;
}

bool Console::cmdSaveGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Save the current game state to the hard disk\n");
		DebugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	int result = 0;
	for (uint i = 0; i < g_EngineState->_fileHandles.size(); i++)
		if (g_EngineState->_fileHandles[i].isOpen())
			result++;

	if (result)
		DebugPrintf("Note: Game state has %d open file handles.\n", result);

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out;
	if (!(out = saveFileMan->openForSaving(argv[1]))) {
		DebugPrintf("Error opening savegame \"%s\" for writing\n", argv[1]);
		return true;
	}

	// TODO: enable custom descriptions? force filename into a specific format?
	if (gamestate_save(g_EngineState, out, "debugging")) {
		DebugPrintf("Saving the game state to '%s' failed\n", argv[1]);
	}

	return true;
}

bool Console::cmdRestoreGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Restores a saved game from the hard disk\n");
		DebugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	EngineState *newstate = NULL;

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::SeekableReadStream *in;
	if (!(in = saveFileMan->openForLoading(argv[1]))) {
		// found a savegame file
		newstate = gamestate_restore(g_EngineState, in);
		delete in;
	}

	if (newstate) {
		g_EngineState->successor = newstate; // Set successor

		script_abort_flag = SCRIPT_ABORT_WITH_REPLAY; // Abort current game
		_debugstate_valid = 0;

		shrink_execution_stack(g_EngineState, g_EngineState->execution_stack_base + 1);
		return 0;
	} else {
		DebugPrintf("Restoring gamestate '%s' failed.\n", argv[1]);
		return 1;
	}

	return false;
}

bool Console::cmdRestartGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Restarts the game. There are two ways to restart a SCI game:\n");
		DebugPrintf("%s play - calls the game object's play() method\n", argv[0]);
		DebugPrintf("%s replay - calls the replay() methody\n", argv[0]);
		return true;
	}

	if (!scumm_stricmp(argv[1], "play")) {
		g_EngineState->restarting_flags |= SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;
	} else if (!scumm_stricmp(argv[1], "replay")) {
		g_EngineState->restarting_flags &= ~SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;
	} else {
		DebugPrintf("Invalid usage of %s\n", argv[0]);
		return true;
	}

	g_EngineState->restarting_flags |= SCI_GAME_IS_RESTARTING_NOW;
	script_abort_flag = 1;
	_debugstate_valid = 0;

	return false;
}

bool Console::cmdClassTable(int argc, const char **argv) {
	DebugPrintf("Available classes:\n");
	for (uint i = 0; i < g_EngineState->_classtable.size(); i++) {
		if (g_EngineState->_classtable[i].reg.segment) {
			DebugPrintf(" Class 0x%x at %04x:%04x (script 0x%x)\n", i, 
					PRINT_REG(g_EngineState->_classtable[i].reg), g_EngineState->_classtable[i].script);
		}
	}

	return true;
}

bool Console::cmdSentenceFragments(int argc, const char **argv) {
	DebugPrintf("Sentence fragments (used to build Parse trees\n");

	for (uint i = 0; i < g_EngineState->_vocabulary->getParserBranchesSize(); i++) {
		int j = 0;

		const parse_tree_branch_t &branch = g_EngineState->_vocabulary->getParseTreeBranch(i);
		DebugPrintf("R%02d: [%x] ->", i, branch.id);
		while ((j < 10) && branch.data[j]) {
			int dat = branch.data[j++];

			switch (dat) {
			case VOCAB_TREE_NODE_COMPARE_TYPE:
				dat = branch.data[j++];
				DebugPrintf(" C(%x)", dat);
				break;

			case VOCAB_TREE_NODE_COMPARE_GROUP:
				dat = branch.data[j++];
				DebugPrintf(" WG(%x)", dat);
				break;

			case VOCAB_TREE_NODE_FORCE_STORAGE:
				dat = branch.data[j++];
				DebugPrintf(" FORCE(%x)", dat);
				break;

			default:
				if (dat > VOCAB_TREE_NODE_LAST_WORD_STORAGE) {
					int dat2 = branch.data[j++];
					DebugPrintf(" %x[%x]", dat, dat2);
				} else
					DebugPrintf(" ?%x?", dat);
			}
		}
		DebugPrintf("\n");
	}

	DebugPrintf("%d rules.\n", g_EngineState->_vocabulary->getParserBranchesSize());

	return true;
}

bool Console::cmdParserNodes(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Shows the specified number of nodes from the parse node tree\n");
		DebugPrintf("Usage: %s <nr>\n", argv[0]);
		DebugPrintf("where <nr> is the number of nodes to show from the parse node tree\n");
		return true;
	}

	int end = MIN<int>(atoi(argv[1]), VOCAB_TREE_NODES);

	for (int i = 0; i < end; i++) {
		DebugPrintf(" Node %03x: ", i);
		if (g_EngineState->parser_nodes[i].type == PARSE_TREE_NODE_LEAF)
			DebugPrintf("Leaf: %04x\n", g_EngineState->parser_nodes[i].content.value);
		else
			DebugPrintf("Branch: ->%04x, ->%04x\n", g_EngineState->parser_nodes[i].content.branches[0],
			          g_EngineState->parser_nodes[i].content.branches[1]);
	}

	return true;
}

bool Console::cmdDrawPic(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Draws a pic resource\n");
		DebugPrintf("Usage: %s <nr> [<pal>] [<fl>]\n", argv[0]);
		DebugPrintf("where <nr> is the number of the pic resource to draw\n");
		DebugPrintf("<pal> is the optional default palette for the pic (default: 0)\n");
		DebugPrintf("<fl> are any pic draw flags (default: 1)\n");
		return true;
	}

	int flags = 1, default_palette = 0;

	if (argc > 2)
		default_palette = atoi(argv[2]);

	if (argc == 4)
		flags = atoi(argv[3]);

	gfxop_new_pic(g_EngineState->gfx_state, atoi(argv[1]), flags, default_palette);
	gfxop_clear_box(g_EngineState->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update(g_EngineState->gfx_state);
	gfxop_sleep(g_EngineState->gfx_state, 0);

	return false;
}

bool Console::cmdDrawRect(int argc, const char **argv) {
	if (argc != 6) {
		DebugPrintf("Draws a rectangle to the screen with one of the EGA colors\n");
		DebugPrintf("Usage: %s <x> <y> <width> <height> <color>\n", argv[0]);
		DebugPrintf("where <color> is the EGA color to use (0-15)\n");
		return true;
	}

	int col = CLIP<int>(atoi(argv[5]), 0, 15);

	gfxop_set_clip_zone(g_EngineState->gfx_state, gfx_rect_fullscreen);
	gfxop_fill_box(g_EngineState->gfx_state, gfx_rect(atoi(argv[1]), atoi(argv[2]), 
										atoi(argv[3]), atoi(argv[4])), g_EngineState->ega_colors[col]);
	gfxop_update(g_EngineState->gfx_state);

	return false;
}

bool Console::cmdDrawCel(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Draws a single view cel to the center of the\n  screen\n\n");
		DebugPrintf("Usage: %s <view> <loop> <cel> <palette>\n", argv[0]);
		return true;
	}

	int view = atoi(argv[1]);
	int loop = atoi(argv[2]);
	int cel = atoi(argv[3]);
	int palette = atoi(argv[4]);

	gfxop_set_clip_zone(g_EngineState->gfx_state, gfx_rect_fullscreen);
	gfxop_draw_cel(g_EngineState->gfx_state, view, loop, cel, Common::Point(160, 100), g_EngineState->ega_colors[0], palette);
	gfxop_update(g_EngineState->gfx_state);

	return false;
}

bool Console::cmdViewInfo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Displays the number of loops and cels of each loop\n");
		DebugPrintf("for the specified view resource and palette.");
		DebugPrintf("Usage: %s <view> <palette>\n", argv[0]);
		return true;
	}

	int view = atoi(argv[1]);
	int palette = atoi(argv[2]);
	int loops, i;
	gfxr_view_t *view_pixmaps = NULL;
	gfx_color_t transparent = { PaletteEntry(), 0, -1, -1, 0 };

	DebugPrintf("Resource view.%d ", view);

	loops = gfxop_lookup_view_get_loops(g_EngineState->gfx_state, view);

	if (loops < 0)
		DebugPrintf("does not exist.\n");
	else {
		DebugPrintf("has %d loops:\n", loops);

		for (i = 0; i < loops; i++) {
			int j, cels;

			DebugPrintf("Loop %d: %d cels.\n", i, cels = gfxop_lookup_view_get_cels(g_EngineState->gfx_state, view, i));
			for (j = 0; j < cels; j++) {
				int width;
				int height;
				Common::Point mod;

				// Show pixmap on screen
				view_pixmaps = g_EngineState->gfx_state->gfxResMan->getView(view, &i, &j, palette);
				gfxop_draw_cel(g_EngineState->gfx_state, view, i, j, Common::Point(0,0), transparent, palette);

				gfxop_get_cel_parameters(g_EngineState->gfx_state, view, i, j, &width, &height, &mod);

				DebugPrintf("   cel %d: size %dx%d, adj+(%d,%d)\n", j, width, height, mod.x, mod.y);
			}
		}
	}

	return true;
}

bool Console::cmdUpdateZone(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Propagates a rectangular area from the back buffer to the front buffer\n");
		DebugPrintf("Usage: %s <x> <y> <width> <height>\n", argv[0]);
		return true;
	}

	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	int width = atoi(argv[3]);
	int height = atoi(argv[4]);

	g_EngineState->gfx_state->driver->update(g_EngineState->gfx_state->driver, gfx_rect(x, y, width, height),
										Common::Point(x, y), GFX_BUFFER_FRONT);

	return false;
}

bool Console::cmdPropagateZone(int argc, const char **argv) {
	if (argc != 5) {
		DebugPrintf("Propagates a rectangular area from a lower graphics buffer to a higher one\n");
		DebugPrintf("Usage: %s <x> <y> <width> <height> <map>\n", argv[0]);
		DebugPrintf("Where <map> can be 0 or 1\n");
		return true;
	}

	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	int width = atoi(argv[3]);
	int height = atoi(argv[4]);
	int map = CLIP<int>(atoi(argv[5]), 0, 1);
	rect_t rect = gfx_rect(x, y, width, height);

	gfxop_set_clip_zone(g_EngineState->gfx_state, gfx_rect_fullscreen);

	if (map == 1)
		gfxop_clear_box(g_EngineState->gfx_state, rect);
	else
		gfxop_update_box(g_EngineState->gfx_state, rect);
	gfxop_update(g_EngineState->gfx_state);
	gfxop_sleep(g_EngineState->gfx_state, 0);

	return false;
}

bool Console::cmdFillScreen(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Fills the screen with one of the EGA colors\n");
		DebugPrintf("Usage: %s <color>\n", argv[0]);
		DebugPrintf("where <color> is the EGA color to use (0-15)\n");
		return true;
	}

	int col = CLIP<int>(atoi(argv[1]), 0, 15);

	gfxop_set_clip_zone(g_EngineState->gfx_state, gfx_rect_fullscreen);
	gfxop_fill_box(g_EngineState->gfx_state, gfx_rect_fullscreen, g_EngineState->ega_colors[col]);
	gfxop_update(g_EngineState->gfx_state);

	return false;
}

bool Console::cmdCurrentPort(int argc, const char **argv) {
	if (!g_EngineState->port)
		DebugPrintf("There is no port active currently.\n");
	else
		DebugPrintf("Current port ID: %d\n", g_EngineState->port->_ID);

	return true;
}

bool Console::cmdPrintPort(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints information about a port\n");
		DebugPrintf("%s current - prints information about the current port\n", argv[0]);
		DebugPrintf("%s <ID> - prints information about the port with the specified ID\n", argv[0]);
		return true;
	}

	GfxPort *port;
	
	if (!scumm_stricmp(argv[1], "current")) {
		port = g_EngineState->port;
		if (!port)
			DebugPrintf("There is no active port currently\n");
		else
			port->print(0);
	} else {
		if (!g_EngineState->visual) {
			DebugPrintf("Visual is uninitialized\n");
		} else {
			port = g_EngineState->visual->getPort(atoi(argv[1]));
			if (!port)
				DebugPrintf("No such port\n");
			else
				port->print(0);
		}
	}

	return true;
}

bool Console::cmdParseGrammar(int argc, const char **argv) {
	DebugPrintf("Parse grammar, in strict GNF:\n");

	g_EngineState->_vocabulary->buildGNF(true);

	return true;
}

bool Console::cmdVisualState(int argc, const char **argv) {
	DebugPrintf("State of the current visual widget:\n");

	if (g_EngineState->visual)
		g_EngineState->visual->print(0);
	else
		DebugPrintf("The visual widget is uninitialized.\n");

	return true;
}

bool Console::cmdFlushPorts(int argc, const char **argv) {
	gfxop_set_pointer_cursor(g_EngineState->gfx_state, GFXOP_NO_POINTER);
	DebugPrintf("Flushing dynamically allocated ports (for memory profiling)...\n");
	delete g_EngineState->visual;
	g_EngineState->gfx_state->gfxResMan->freeAllResources();
	g_EngineState->visual = NULL;

	return true;
}

bool Console::cmdDynamicViews(int argc, const char **argv) {
	DebugPrintf("List of active dynamic views:\n");

	if (g_EngineState->dyn_views)
		g_EngineState->dyn_views->print(0);
	else
		DebugPrintf("The list is empty.\n");

	return true;
}

bool Console::cmdDroppedViews(int argc, const char **argv) {
	DebugPrintf("List of dropped dynamic views:\n");

	if (g_EngineState->drop_views)
		g_EngineState->drop_views->print(0);
	else
		DebugPrintf("The list is empty.\n");

	return true;
}

bool Console::cmdPriorityBands(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Priority bands start at y=%d. They end at y=%d\n", g_EngineState->priority_first, g_EngineState->priority_last);
		DebugPrintf("Use %d <priority band> to print the start of priority for the specified priority band (0 - 15)\n", argv[0]);
		return true;
	}

	int zone = CLIP<int>(atoi(argv[1]), 0, 15);
	DebugPrintf("Zone %x starts at y=%d\n", zone, _find_priority_band(g_EngineState, zone));

	return true;
}

bool Console::cmdStatusBarColors(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Sets the colors of the status bar\n");
		DebugPrintf("Usage: %s <foreground color> <background color>\n", argv[0]);
		return true;
	}

	g_EngineState->titlebar_port->_color = g_EngineState->ega_colors[atoi(argv[1])];
	g_EngineState->titlebar_port->_bgcolor = g_EngineState->ega_colors[atoi(argv[2])];

	g_EngineState->status_bar_foreground = atoi(argv[1]);
	g_EngineState->status_bar_background = atoi(argv[2]);

	sciw_set_status_bar(g_EngineState, g_EngineState->titlebar_port, g_EngineState->_statusBarText, 
							g_EngineState->status_bar_foreground, g_EngineState->status_bar_background);
	gfxop_update(g_EngineState->gfx_state);

	return false;
}

bool Console::cmdSimulateKey(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Simulate a keypress with the specified scancode\n");
		DebugPrintf("Usage: %s <key scan code>\n", argv[0]);
		return true;
	}

	_kdebug_cheap_event_hack = atoi(argv[1]);

	return true;
}

bool Console::cmdTrackMouse(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Toggles mouse position tracking\n");
		DebugPrintf("Usage: %s <on/off>\n", argv[0]);
		DebugPrintf("If switched on, left mouse clicks will print\n");
		DebugPrintf("the coordinates clicked in the debug console\n");
		return true;
	}

	if (!scumm_stricmp(argv[1], "on")) {
		_kdebug_track_mouse_clicks = true;
		DebugPrintf("Mouse tracking turned on\n");
	} else if (!scumm_stricmp(argv[1], "off")) {
		_kdebug_track_mouse_clicks = false;
		DebugPrintf("Mouse tracking turned off\n");
	}

	return true;
}

bool Console::cmdPrintSegmentTable(int argc, const char **argv) {
	DebugPrintf("Segment table:\n");

	for (uint i = 0; i < g_EngineState->seg_manager->_heap.size(); i++) {
		MemObject *mobj = g_EngineState->seg_manager->_heap[i];
		if (mobj && mobj->getType()) {
			DebugPrintf(" [%04x] ", i);

			switch (mobj->getType()) {
			case MEM_OBJ_SCRIPT:
				DebugPrintf("S  script.%03d l:%d ", (*(Script *)mobj).nr, (*(Script *)mobj).lockers);
				break;

			case MEM_OBJ_CLONES:
				DebugPrintf("C  clones (%d allocd)", (*(CloneTable *)mobj).entries_used);
				break;

			case MEM_OBJ_LOCALS:
				DebugPrintf("V  locals %03d", (*(LocalVariables *)mobj).script_id);
				break;

			case MEM_OBJ_STACK:
				DebugPrintf("D  data stack (%d)", (*(DataStack *)mobj).nr);
				break;

			case MEM_OBJ_SYS_STRINGS:
				DebugPrintf("Y  system string table");
				break;

			case MEM_OBJ_LISTS:
				DebugPrintf("L  lists (%d)", (*(ListTable *)mobj).entries_used);
				break;

			case MEM_OBJ_NODES:
				DebugPrintf("N  nodes (%d)", (*(NodeTable *)mobj).entries_used);
				break;

			case MEM_OBJ_HUNK:
				DebugPrintf("H  hunk (%d)", (*(HunkTable *)mobj).entries_used);
				break;

			case MEM_OBJ_DYNMEM:
				DebugPrintf("M  dynmem: %d bytes", (*(DynMem *)mobj)._size);
				break;

			case MEM_OBJ_STRING_FRAG:
				DebugPrintf("F  string fragments");
				break;

			default:
				DebugPrintf("I  Invalid (type = %x)", mobj->getType());
				break;
			}

			DebugPrintf("  seg_ID = %d \n", mobj->getSegMgrId());
		}
	}
	DebugPrintf("\n");

	return true;
}

bool Console::segmentInfo(int nr) {
	DebugPrintf("[%04x] ", nr);

	if ((nr < 0) || ((uint)nr >= g_EngineState->seg_manager->_heap.size()) || !g_EngineState->seg_manager->_heap[nr])
		return false;

	MemObject *mobj = g_EngineState->seg_manager->_heap[nr];

	switch (mobj->getType()) {

	case MEM_OBJ_SCRIPT: {
		Script *scr = (Script *)mobj;
		DebugPrintf("script.%03d locked by %d, bufsize=%d (%x)\n", scr->nr, scr->lockers, (uint)scr->buf_size, (uint)scr->buf_size);
		if (scr->export_table)
			DebugPrintf("  Exports: %4d at %d\n", scr->exports_nr, (int)(((byte *)scr->export_table) - ((byte *)scr->buf)));
		else
			DebugPrintf("  Exports: none\n");

		DebugPrintf("  Synonyms: %4d\n", scr->synonyms_nr);

		if (scr->locals_block)
			DebugPrintf("  Locals : %4d in segment 0x%x\n", scr->locals_block->_locals.size(), scr->locals_segment);
		else
			DebugPrintf("  Locals : none\n");

		DebugPrintf("  Objects: %4d\n", scr->_objects.size());
		for (uint i = 0; i < scr->_objects.size(); i++) {
			DebugPrintf("    ");
			// Object header
			Object *obj = obj_get(g_EngineState, scr->_objects[i].pos);
			if (obj)
				DebugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(scr->_objects[i].pos), 
							obj_get_name(g_EngineState, scr->_objects[i].pos), obj->_variables.size(), obj->methods_nr);
		}
	}
	break;

	case MEM_OBJ_LOCALS: {
		LocalVariables *locals = (LocalVariables *)mobj;
		DebugPrintf("locals for script.%03d\n", locals->script_id);
		DebugPrintf("  %d (0x%x) locals\n", locals->_locals.size(), locals->_locals.size());
	}
	break;

	case MEM_OBJ_STACK: {
		DataStack *stack = (DataStack *)mobj;
		DebugPrintf("stack\n");
		DebugPrintf("  %d (0x%x) entries\n", stack->nr, stack->nr);
	}
	break;

	case MEM_OBJ_SYS_STRINGS: {
		DebugPrintf("system string table - viewing currently disabled\n");
#if 0
		SystemStrings *strings = &(mobj->data.sys_strings);

		for (int i = 0; i < SYS_STRINGS_MAX; i++)
			if (strings->strings[i].name)
				DebugPrintf("  %s[%d]=\"%s\"\n", strings->strings[i].name, strings->strings[i].max_size, strings->strings[i].value);
#endif
	}
	break;

	case MEM_OBJ_CLONES: {
		CloneTable *ct = (CloneTable *)mobj;

		DebugPrintf("clones\n");

		for (uint i = 0; i < ct->_table.size(); i++)
			if (ct->isValidEntry(i)) {
				reg_t objpos;
				objpos.offset = i;
				objpos.segment = nr;
				DebugPrintf("  [%04x] %s; copy of ", i, obj_get_name(g_EngineState, objpos));
				// Object header
				Object *obj = obj_get(g_EngineState, ct->_table[i].pos);
				if (obj)
					DebugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(ct->_table[i].pos), 
								obj_get_name(g_EngineState, ct->_table[i].pos), obj->_variables.size(), obj->methods_nr);
			}
	}
	break;

	case MEM_OBJ_LISTS: {
		ListTable *lt = (ListTable *)mobj;

		DebugPrintf("lists\n");
		for (uint i = 0; i < lt->_table.size(); i++)
			if (lt->isValidEntry(i)) {
				DebugPrintf("  [%04x]: ", i);
				printList(&(lt->_table[i]));
			}
	}
	break;

	case MEM_OBJ_NODES: {
		DebugPrintf("nodes (total %d)\n", (*(NodeTable *)mobj).entries_used);
		break;
	}

	case MEM_OBJ_HUNK: {
		HunkTable *ht = (HunkTable *)mobj;

		DebugPrintf("hunk  (total %d)\n", ht->entries_used);
		for (uint i = 0; i < ht->_table.size(); i++)
			if (ht->isValidEntry(i)) {
				DebugPrintf("    [%04x] %d bytes at %p, type=%s\n",
				          i, ht->_table[i].size, ht->_table[i].mem, ht->_table[i].type);
			}
	}
	break;

	case MEM_OBJ_DYNMEM: {
		DebugPrintf("dynmem (%s): %d bytes\n",
		          (*(DynMem *)mobj)._description ? (*(DynMem *)mobj)._description : "no description", (*(DynMem *)mobj)._size);

		Common::hexdump((*(DynMem *)mobj)._buf, (*(DynMem *)mobj)._size, 16, 0);
	}
	break;

	case MEM_OBJ_STRING_FRAG: {
		DebugPrintf("string frags\n");
		break;
	}

	default :
		DebugPrintf("Invalid type %d\n", mobj->getType());
		break;
	}

	DebugPrintf("\n");
	return true;
}

void Console::printList(List *l) {
	reg_t pos = l->first;
	reg_t my_prev = NULL_REG;

	DebugPrintf("\t<\n");

	while (!pos.isNull()) {
		Node *node;
		NodeTable *nt = (NodeTable *)GET_SEGMENT(*g_EngineState->seg_manager, pos.segment, MEM_OBJ_NODES);

		if (!nt || !nt->isValidEntry(pos.offset)) {
			DebugPrintf("   WARNING: %04x:%04x: Doesn't contain list node!\n",
			          PRINT_REG(pos));
			return;
		}

		node = &(nt->_table[pos.offset]);

		DebugPrintf("\t%04x:%04x  : %04x:%04x -> %04x:%04x\n", PRINT_REG(pos), PRINT_REG(node->key), PRINT_REG(node->value));

		if (my_prev != node->pred)
			DebugPrintf("   WARNING: current node gives %04x:%04x as predecessor!\n",
			          PRINT_REG(node->pred));

		my_prev = pos;
		pos = node->succ;
	}

	if (my_prev != l->last)
		DebugPrintf("   WARNING: Last node was expected to be %04x:%04x, was %04x:%04x!\n",
		          PRINT_REG(l->last), PRINT_REG(my_prev));
	DebugPrintf("\t>\n");
}

bool Console::cmdSegmentInfo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Provides information on the specified segment(s)\n");
		DebugPrintf("Usage: %s <segment number>\n", argv[0]);
		DebugPrintf("<segment number> can be a number, which shows the information of the segment with\n");
		DebugPrintf("the specified number, or \"all\" to show information on all active segments");
		return true;
	}

	if (!scumm_stricmp(argv[1], "all")) {
		for (uint i = 0; i < g_EngineState->seg_manager->_heap.size(); i++)
			segmentInfo(i);
	} else {
		int nr = atoi(argv[1]);
		if (!segmentInfo(nr))
			DebugPrintf("Segment %04x does not exist\n", nr);
	}

	return true;
}


bool Console::cmdKillSegment(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Deletes the specified segment\n");
		DebugPrintf("Usage: %s <segment number>\n", argv[0]);
		return true;
	}

	g_EngineState->seg_manager->getScript(atoi(argv[1]))->setLockers(0);

	return true;
}

bool Console::cmdShowMap(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Shows one of the screen maps\n");
		DebugPrintf("Usage: %s <screen map>\n", argv[0]);
		DebugPrintf("Screen maps:\n");
		DebugPrintf("- 0: visual map (back buffer)\n");
		DebugPrintf("- 1: priority map (back buffer)\n");
		DebugPrintf("- 2: control map (static buffer)\n");
		return true;
	}

	gfxop_set_clip_zone(g_EngineState->gfx_state, gfx_rect_fullscreen);

	int map = atoi(argv[1]);

	switch (map) {
	case 0:
		g_EngineState->visual->add_dirty_abs((GfxContainer *)g_EngineState->visual, gfx_rect(0, 0, 320, 200), 0);
		g_EngineState->visual->draw(Common::Point(0, 0));
		break;

	case 1:
		gfx_xlate_pixmap(g_EngineState->gfx_state->pic->priority_map, g_EngineState->gfx_state->driver->mode, GFX_XLATE_FILTER_NONE);
		gfxop_draw_pixmap(g_EngineState->gfx_state, g_EngineState->gfx_state->pic->priority_map, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		break;

	case 2:
		gfx_xlate_pixmap(g_EngineState->gfx_state->control_map, g_EngineState->gfx_state->driver->mode, GFX_XLATE_FILTER_NONE);
		gfxop_draw_pixmap(g_EngineState->gfx_state, g_EngineState->gfx_state->control_map, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		break;

	default:
		DebugPrintf("Map %d is not available.\n", map);
		return true;
	}

	gfxop_update(g_EngineState->gfx_state);

	return false;
}

bool Console::cmdSongLib(int argc, const char **argv) {
	DebugPrintf("Song library:\n");

	song_t *seeker = *(g_EngineState->_sound._songlib.lib);

	do {
		DebugPrintf("    %p", (void *)seeker);

		if (seeker) {
			DebugPrintf("[%04lx,p=%d,s=%d]->", seeker->handle, seeker->priority, seeker->status);
			seeker = seeker->next;
		}
		DebugPrintf("\n");
	} while (seeker);
	DebugPrintf("\n");

	return true;
}

bool Console::cmdGCInvoke(int argc, const char **argv) {
	DebugPrintf("Performing garbage collection...\n");
	run_gc(g_EngineState);
	return true;
}

bool Console::cmdGCObjects(int argc, const char **argv) {
	reg_t_hash_map *use_map = find_all_used_references(g_EngineState);

	DebugPrintf("Reachable object references (normalised):\n");
	for (reg_t_hash_map::iterator i = use_map->begin(); i != use_map->end(); ++i) {
		DebugPrintf(" - %04x:%04x\n", PRINT_REG(i->_key));
	}

	delete use_map;

	return true;
}

bool Console::cmdGCInterval(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Sets the number of kernel calls in between garbage collections.\n");
		DebugPrintf("Usage: %s <number>\n", argv[0]);
		DebugPrintf("Number of kernel calls in between garbage collections is currently %d.", script_gc_interval);
		return true;
	}

	DebugPrintf("Number of kernel calls in between garbage collections was %d, setting it to %d\n", script_gc_interval, atoi(argv[1]));
	script_gc_interval = atoi(argv[1]);

	return true;
}

// TODO/FIXME: This should be using DebugPrintf
void _print_address(void * _, reg_t addr) {
	if (addr.segment)
		sciprintf("  %04x:%04x\n", PRINT_REG(addr));
}

bool Console::cmdGCShowReachable(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints all addresses directly reachable from the memory object specified as parameter.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;
	
	if (parse_reg_t(g_EngineState, argv[1], &addr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	MemObject *mobj = GET_SEGMENT_ANY(*g_EngineState->seg_manager, addr.segment);
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.segment);
		return 1;
	}

	DebugPrintf("Reachable from %04x:%04x:\n", PRINT_REG(addr));
	mobj->listAllOutgoingReferences(g_EngineState, addr, NULL, _print_address);

	return true;
}

bool Console::cmdGCShowFreeable(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints all addresses freeable in the segment associated with the\n");
		DebugPrintf("given address (offset is ignored).\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;
	
	if (parse_reg_t(g_EngineState, argv[1], &addr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	MemObject *mobj = GET_SEGMENT_ANY(*g_EngineState->seg_manager, addr.segment);
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.segment);
		return true;
	}

	DebugPrintf("Freeable in segment %04x:\n", addr.segment);
	mobj->listAllDeallocatable(addr.segment, NULL, _print_address);

	return true;
}

bool Console::cmdGCNormalize(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints the \"normal\" address of a given address,\n");
		DebugPrintf("i.e. the address we would free in order to free\n");
		DebugPrintf("the object associated with the original address.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;
	
	if (parse_reg_t(g_EngineState, argv[1], &addr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	MemObject *mobj = GET_SEGMENT_ANY(*g_EngineState->seg_manager, addr.segment);
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.segment);
		return true;
	}

	addr = mobj->findCanonicAddress(g_EngineState->seg_manager, addr);
	DebugPrintf(" %04x:%04x\n", PRINT_REG(addr));

	return true;
}

bool Console::cmdVMVarlist(int argc, const char **argv) {
	//const char *varnames[] = {"global", "local", "temp", "param"};

	DebugPrintf("Addresses of variables in the VM:\n");

#if 0
	// TODO: p_var_segs, p_vars, p_var_base, p_var_max

	for (int i = 0; i < 4; i++) {
		DebugPrintf("%s vars at %04x:%04x ", varnames[i], PRINT_REG(make_reg(p_var_segs[i], p_vars[i] - p_var_base[i])));
		if (p_var_max)
			DebugPrintf("  total %d", p_var_max[i]);
		DebugPrintf("\n");
	}
#endif

	return true;
}

bool Console::cmdStack(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Dumps the specified number of stack elements.\n");
		DebugPrintf("Usage: %s <elements>\n", argv[0]);
		return true;
	}

	if (g_EngineState->_executionStack.empty()) {
		DebugPrintf("No exec stack!");
		return true;
	}

	ExecStack &xs = g_EngineState->_executionStack.back();
	int nr = atoi(argv[1]);

	for (int i = nr; i > 0; i--) {
		if ((xs.sp - xs.fp - i) == 0)
			DebugPrintf("-- temp variables --\n");
		if (xs.sp - i >= g_EngineState->stack_base)
			DebugPrintf("ST:%04x = %04x:%04x\n", (unsigned)(xs.sp - i - g_EngineState->stack_base), PRINT_REG(xs.sp[-i]));
	}

	return true;
}

bool Console::cmdValueType(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Determines the type of a value.\n");
		DebugPrintf("The type can be one of the following:\n");
		DebugPrintf("Invalid, list, object, reference or arithmetic\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t val;
	
	if (parse_reg_t(g_EngineState, argv[1], &val)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	int t = determine_reg_type(g_EngineState, val, 1);
	int invalid = t & KSIG_INVALID;

	switch (t & ~KSIG_INVALID) {
	case 0:
		DebugPrintf("Invalid");
		break;
	case KSIG_LIST:
		DebugPrintf("List");
		break;
	case KSIG_OBJECT:
		DebugPrintf("Object");
		break;
	case KSIG_REF:
		DebugPrintf("Reference");
		break;
	case KSIG_ARITHMETIC:
		DebugPrintf("Arithmetic");
		break;
	default:
		DebugPrintf("Erroneous unknown type %02x(%d decimal)\n", t, t);
	}

	DebugPrintf("%s\n", invalid ? " (invalid)" : "");

	return true;
}

bool Console::cmdSleepFactor(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Factor to multiply with wait times in kWait().\n");
		DebugPrintf("Set to 0 to speed up games.\n");
		DebugPrintf("Usage: %s <factor>\n", argv[0]);
		DebugPrintf("Sleep factor is currently %d.", debug_sleeptime_factor);
		return true;
	}

	DebugPrintf("Sleep factor was %d, setting it to %d\n", debug_sleeptime_factor, atoi(argv[1]));
	debug_sleeptime_factor = atoi(argv[1]);

	return true;
}

bool Console::cmdScriptSteps(int argc, const char **argv) {
	DebugPrintf("Number of executed SCI operations: %d\n", script_step_counter);
	return true;
}

bool Console::cmdSetAccumulator(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Sets the accumulator.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t val;
	
	if (parse_reg_t(g_EngineState, argv[1], &val)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	g_EngineState->r_acc = val;

	return true;
}

bool Console::cmdIsSample(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Tests whether a given sound resource is a PCM sample, \n");
		DebugPrintf("and displays information on it if it is.\n");
		DebugPrintf("Usage: %s <sample id>\n", argv[0]);
		return true;
	}

	Resource *song = _vm->getResMgr()->findResource(kResourceTypeSound, atoi(argv[1]), 0);
	SongIterator *songit;
	Audio::AudioStream *data;

	if (!song) {
		DebugPrintf("Not a sound resource!\n");
		return true;
	}

	songit = songit_new(song->data, song->size, SCI_SONG_ITERATOR_TYPE_SCI0, 0xcaffe /* What do I care about the ID? */);

	if (!songit) {
		DebugPrintf("Could not convert to song iterator!\n");
		return true;
	}

	if ((data = songit->getAudioStream())) {
		// TODO
/*
		DebugPrintf("\nIs sample (encoding %dHz/%s/%04x)", data->conf.rate, (data->conf.stereo) ?
		          ((data->conf.stereo == SFX_PCM_STEREO_LR) ? "stereo-LR" : "stereo-RL") : "mono", data->conf.format);
*/
		delete data;
	} else
		DebugPrintf("Valid song, but not a sample.\n");

	delete songit;

	return true;
}

bool Console::cmdSfx01Header(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Dumps the header of a SCI01 song\n");
		DebugPrintf("Usage: %s <track>\n", argv[0]);
		return true;
	}

	Resource *song = _vm->getResMgr()->findResource(kResourceTypeSound, atoi(argv[1]), 0);

	if (!song) {
		DebugPrintf("Doesn't exist\n");
		return true;
	}

	uint32 offset = 0;

	DebugPrintf("SCI01 song track mappings:\n");

	if (*song->data == 0xf0) // SCI1 priority spec
		offset = 8;

	if (song->size <= 0)
		return 1;

	while (song->data[offset] != 0xff) {
		byte device_id = song->data[offset];
		DebugPrintf("* Device %02x:\n", device_id);
		offset++;

		if (offset + 1 >= song->size)
			return 1;

		while (song->data[offset] != 0xff) {
			int track_offset;
			int end;
			byte header1, header2;

			if (offset + 7 >= song->size)
				return 1;

			offset += 2;

			track_offset = READ_LE_UINT16(song->data + offset);
			header1 = song->data[track_offset];
			header2 = song->data[track_offset+1];
			track_offset += 2;

			end = READ_LE_UINT16(song->data + offset + 2);
			DebugPrintf("  - %04x -- %04x", track_offset, track_offset + end);

			if (track_offset == 0xfe)
				DebugPrintf(" (PCM data)\n");
			else
				DebugPrintf(" (channel %d, special %d, %d playing notes, %d foo)\n",
				          header1 & 0xf, header1 >> 4, header2 & 0xf, header2 >> 4);
			offset += 4;
		}
		offset++;
	}

	return true;
}

static int _parse_ticks(byte *data, int *offset_p, int size) {
	int ticks = 0;
	int tempticks;
	int offset = 0;

	do {
		tempticks = data[offset++];
		ticks += (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX) ? SCI_MIDI_TIME_EXPANSION_LENGTH : tempticks;
	} while (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX && offset < size);

	if (offset_p)
		*offset_p = offset;

	return ticks;
}

// Specialised for SCI01 tracks (this affects the way cumulative cues are treated)
static void midi_hexdump(byte *data, int size, int notational_offset) {
	int offset = 0;
	int prev = 0;
	const int MIDI_cmdlen[16] = {0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0};

	if (*data == 0xf0) // SCI1 priority spec
		offset = 8;

	while (offset < size) {
		int old_offset = offset;
		int offset_mod;
		int time = _parse_ticks(data + offset, &offset_mod, size);
		int cmd;
		int pleft;
		int firstarg = 0;
		int i;
		int blanks = 0;

		offset += offset_mod;
		printf("  [%04x] %d\t",
		        old_offset + notational_offset, time);

		cmd = data[offset];
		if (!(cmd & 0x80)) {
			cmd = prev;
			if (prev < 0x80) {
				printf("Track broken at %x after"
				        " offset mod of %d\n",
				        offset + notational_offset, offset_mod);
				Common::hexdump(data, size, 16, notational_offset);
				return;
			}
			printf("(rs %02x) ", cmd);
			blanks += 8;
		} else {
			++offset;
			printf("%02x ", cmd);
			blanks += 3;
		}
		prev = cmd;

		pleft = MIDI_cmdlen[cmd >> 4];
		if (SCI_MIDI_CONTROLLER(cmd) && data[offset] == SCI_MIDI_CUMULATIVE_CUE)
			--pleft; // This is SCI(0)1 specific

		for (i = 0; i < pleft; i++) {
			if (i == 0)
				firstarg = data[offset];
			printf("%02x ", data[offset++]);
			blanks += 3;
		}

		while (blanks < 16) {
			blanks += 4;
			printf("    ");
		}

		while (blanks < 20) {
			++blanks;
			printf(" ");
		}

		if (cmd == SCI_MIDI_EOT)
			printf(";; EOT");
		else if (cmd == SCI_MIDI_SET_SIGNAL) {
			if (firstarg == SCI_MIDI_SET_SIGNAL_LOOP)
				printf(";; LOOP point");
			else
				printf(";; CUE (%d)", firstarg);
		} else if (SCI_MIDI_CONTROLLER(cmd)) {
			if (firstarg == SCI_MIDI_CUMULATIVE_CUE)
				printf(";; CUE (cumulative)");
			else if (firstarg == SCI_MIDI_RESET_ON_SUSPEND)
				printf(";; RESET-ON-SUSPEND flag");
		}
		printf("\n");

		if (old_offset >= offset) {
			printf("-- Not moving forward anymore,"
			        " aborting (%x/%x)\n", offset, old_offset);
			return;
		}
	}
}

bool Console::cmdSfx01Track(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Dumps a track of a SCI01 song\n");
		DebugPrintf("Usage: %s <track> <offset>\n", argv[0]);
		return true;
	}

	Resource *song = _vm->getResMgr()->findResource(kResourceTypeSound, atoi(argv[1]), 0);

	int offset = atoi(argv[2]);

	if (!song) {
		DebugPrintf("Doesn't exist\n");
		return true;
	}

	midi_hexdump(song->data + offset, song->size, offset);

	return true;
}

bool Console::cmdStopSfx(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Stops a playing sound\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Where <address> is the address of the sound to stop.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t id;
	
	if (parse_reg_t(g_EngineState, argv[1], &id)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	int handle = id.segment << 16 | id.offset;	// frobnicate handle
	EngineState* s = g_EngineState;		// for PUT_SEL32V

	if (id.segment) {
		g_EngineState->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
		g_EngineState->_sound.sfx_remove_song(handle);
		PUT_SEL32V(id, signal, -1);
		PUT_SEL32V(id, nodePtr, 0);
		PUT_SEL32V(id, handle, 0);
	}

	return true;
}

bool Console::cmdExit(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("%s game - exit gracefully\n", argv[0]);
		DebugPrintf("%s now - exit ungracefully\n", argv[0]);
		return true;
	}

	if (!scumm_stricmp(argv[1], "game")) {
		// Quit gracefully
		script_abort_flag = 1; // Terminate VM
		_debugstate_valid = 0;
		_debug_seeking = 0;
		_debug_step_running = 0;

	} else if (!scumm_stricmp(argv[1], "now")) {
		// Quit ungracefully
		exit(0);
	}

	return false;
}

bool Console::cmdAddresses(int argc, const char **argv) {
	DebugPrintf("Address parameters may be passed in one of three forms:\n");
	DebugPrintf(" - ssss:oooo -- where 'ssss' denotes a segment and 'oooo' an offset.\n");
	DebugPrintf("   Example: \"a:c5\" would address something in segment 0xa at offset 0xc5.\n");
	DebugPrintf(" - &scr:oooo -- where 'scr' is a script number and oooo an offset within that script; will\n");
	DebugPrintf("   fail if the script is not currently loaded\n");
	DebugPrintf(" - $REG -- where 'REG' is one of 'PC', 'ACC', 'PREV' or 'OBJ': References the address\n");
	DebugPrintf("   indicated by the register of this name.\n");
	DebugPrintf(" - $REG+n (or -n) -- Like $REG, but modifies the offset part by a specific amount (which\n");
	DebugPrintf("   is specified in hexadecimal).\n");
	DebugPrintf(" - ?obj -- Looks up an object with the specified name, uses its address. This will abort if\n");
	DebugPrintf("   the object name is ambiguous; in that case, a list of addresses and indices is provided.\n");
	DebugPrintf("   ?obj.idx may be used to disambiguate 'obj' by the index 'idx'.\n");

	return true;
}

} // End of namespace Sci
