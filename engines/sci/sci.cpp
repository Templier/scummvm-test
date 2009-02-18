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
#include "common/config-manager.h"

#include "engines/advancedDetector.h"
#include "sci/sci.h"
#include "sci/include/engine.h"

//namespace Sci {

extern gfx_driver_t gfx_driver_scummvm;

int
c_quit(state_t *s) {
	script_abort_flag = 1; /* Terminate VM */
	_debugstate_valid = 0;
	_debug_seeking = 0;
	_debug_step_running = 0;
	return 0;
}

int
c_die(state_t *s) {
	exit(0); /* Die */
	return 0; /* ;-P (fixes warning) */
}

static void
init_console() {
#ifdef WANT_CONSOLE
	con_gfx_init();
#endif
	con_hook_command(&c_quit, "quit", "", "console: Quits gracefully");
	con_hook_command(&c_die, "die", "", "console: Quits ungracefully");

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
	con_hook_int(&gfx_crossblit_alpha_threshold, "alpha_threshold",
	             "Alpha threshold for crossblitting\n");
	con_hook_int(&sci0_palette, "sci0_palette",
	             "SCI0 palette- 0: EGA, 1:AGI/Amiga, 2:Grayscale\n");
	con_hook_int(&sci01_priority_table_flags, "sci01_priority_table_flags",
	             "SCI01 priority table debugging flags: 1:Disable, 2:Print on change\n");

	con_passthrough = 1; /* enables all sciprintf data to be sent to stdout */
}

static int
init_gamestate(state_t *gamestate, sci_version_t version) {
	int errc;

	if ((errc = script_init_engine(gamestate, version))) { /* Initialize game state */
		int recovered = 0;

		if (errc == SCI_ERROR_INVALID_SCRIPT_VERSION) {
			int tversion = SCI_VERSION_FTU_NEW_SCRIPT_HEADER - ((version < SCI_VERSION_FTU_NEW_SCRIPT_HEADER) ? 0 : 1);

			while (!recovered && tversion) {
				printf("Trying version %d.%03x.%03d instead\n", SCI_VERSION_MAJOR(tversion),
				       SCI_VERSION_MINOR(tversion), SCI_VERSION_PATCHLEVEL(tversion));

				errc = script_init_engine(gamestate, tversion);

				if ((recovered = !errc))
					version = tversion;

				if (errc != SCI_ERROR_INVALID_SCRIPT_VERSION)
					break;

				switch (tversion) {

				case SCI_VERSION_FTU_NEW_SCRIPT_HEADER - 1:
					if (version >= SCI_VERSION_FTU_NEW_SCRIPT_HEADER)
						tversion = 0;
					else
						tversion = SCI_VERSION_FTU_NEW_SCRIPT_HEADER;
					break;

				case SCI_VERSION_FTU_NEW_SCRIPT_HEADER:
					tversion = 0;
					break;
				}
			}
			if (recovered)
				printf("Success.\n");
		}

		if (!recovered) {
			fprintf(stderr, "Script initialization failed. Aborting...\n");
			return 1;
		}
	}
	return 0;
}

SciEngine::SciEngine(OSystem *syst, const SciGameDescription *desc)
		: Engine(syst) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().

	// However this is the place to specify all default directories
	//File::addDefaultDirectory(_gameDataPath + "sound/");
	//printf("%s\n", _gameDataPath.c_str());

	// Set up the engine specific debug levels
	//Common::addSpecialDebugLevel(SCI_DEBUG_RESOURCES, "resources", "Debug the resources loading");

	_version = desc->version;
	printf("SciEngine::SciEngine\n");
}

SciEngine::~SciEngine() {
	// Dispose your resources here
	printf("SciEngine::~SciEngine\n");

	// Remove all of our debug levels here
	//Common::clearAllSpecialDebugLevels();
}

Common::Error SciEngine::init() {
	initGraphics(320, 200, false);

	//GUIErrorMessage("lalalal asfa w4 haha hreh au u<w");

	// Create debugger console. It requires GFX to be initialized
	//_console = new Console(this);
	//_console = new Console();

	// Additional setup.
	printf("SciEngine::init\n");
	return Common::kNoError;
}

Common::Error SciEngine::go() {
	// Your main even loop should be (invoked from) here.

	/* bool end = false;
	Common::EventManager *em = _system->getEventManager();
	while (!end) {
		Common::Event ev;
		if (em->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				end = true;
			}
		}
		_system->delayMillis(10);
	} */

	// FIXME/TODO: Move some of the stuff below to init()
	resource_mgr_t *resmgr;

	init_console(); /* So we can get any output */

	script_debug_flag = 0;

	sci_version_t version;
	int res_version = SCI_VERSION_AUTODETECT;

	// FIXME. An evil hack until File class will be used properly
	chdir(ConfMan.get("path").c_str());

	version = getVersion();

	char resource_dir[MAXPATHLEN+1] = "";
	getcwd(resource_dir, MAXPATHLEN); /* Store resource directory */

	resmgr = scir_new_resource_manager(resource_dir, res_version, 1, 256 * 1024);

	if (!resmgr) {
		printf("No resources found in '%s'.\nAborting...\n",
		       resource_dir);
		return Common::kNoGameDataFoundError;
	}

	script_adjust_opcode_formats(resmgr->sci_version);

#if 0
	printf("Mapping instruments to General Midi\n");

	map_MIDI_instruments(resmgr);
#endif

	sciprintf("Imported FreeSCI, version "VERSION"\n");

	state_t* gamestate = (state_t *) sci_malloc(sizeof(state_t));
	memset(gamestate, 0, sizeof(state_t));
	gamestate->resmgr = resmgr;
	gamestate->gfx_state = NULL;

	if (init_gamestate(gamestate, version))
		return Common::kUnknownError;


	if (game_init(gamestate)) { /* Initialize */
		fprintf(stderr, "Game initialization failed: Aborting...\n");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	/* Set the savegame dir */
	script_set_gamestate_save_dir(gamestate, ConfMan.get("savepath").c_str());

	// Originally, work_dir tried to be ~/.freesci/game_name
	gamestate->work_dir = sci_strdup(ConfMan.get("savepath").c_str());
	gamestate->resource_dir = resource_dir;
	gamestate->port_serial = 0;
	gamestate->have_mouse_flag = 1;
	gamestate->animation_delay = 5;
	gamestate->animation_granularity = 4;
	gfx_crossblit_alpha_threshold = 0x90;

	gfx_state_t gfx_state;
	gfx_state.driver = &gfx_driver_scummvm;
	gfx_state.version = resmgr->sci_version;
	gamestate->gfx_state = &gfx_state;

	/**** Default config: */
	gfx_options_t gfx_options;
	gfx_options.workarounds = 0;
	gfx_options.buffer_pics_nr = 0;
	gfx_options.correct_rendering = 1;
	gfx_options.pic0_unscaled = 1;
	gfx_options.pic0_dither_mode = GFXR_DITHER_MODE_D256;
	gfx_options.pic0_dither_pattern = GFXR_DITHER_PATTERN_SCALED;
	gfx_options.pic0_brush_mode = GFX_BRUSH_MODE_RANDOM_ELLIPSES;
	gfx_options.pic0_line_mode = GFX_LINE_MODE_CORRECT;
	gfx_options.cursor_xlate_filter = GFX_XLATE_FILTER_NONE;
	gfx_options.view_xlate_filter = GFX_XLATE_FILTER_NONE;
	gfx_options.pic_xlate_filter = GFX_XLATE_FILTER_NONE;
	gfx_options.text_xlate_filter = GFX_XLATE_FILTER_NONE;
	gfx_options.dirty_frames = GFXOP_DIRTY_FRAMES_CLUSTERS;
	gfx_options.pic0_antialiasing = GFXR_ANTIALIASING_NONE;
	gfx_options.pic_port_bounds = gfx_rect(0, 10, 320, 190);
	for (int i = 0; i < GFX_RESOURCE_TYPES_NR; i++) {
		gfx_options.res_conf.assign[i] = NULL;
		gfx_options.res_conf.mod[i] = NULL;
	}
	/**** Default config ends */

	if (gfxop_init_default(&gfx_state, &gfx_options, resmgr)) {
		fprintf(stderr, "Graphics initialization failed. Aborting...\n");
		return Common::kUnknownError;
	}

	if (game_init_graphics(gamestate)) { /* Init interpreter graphics */
		fprintf(stderr, "Game initialization failed: Error in GFX subsystem. Aborting...\n");
		return Common::kUnknownError;
	}

	if (game_init_sound(gamestate, 0)) {
		fprintf(stderr, "Game initialization failed: Error in sound subsystem. Aborting...\n");
		return Common::kUnknownError;
	}

	printf("Emulating SCI version %d.%03d.%03d\n",
	       SCI_VERSION_MAJOR(gamestate->version),
	       SCI_VERSION_MINOR(gamestate->version),
	       SCI_VERSION_PATCHLEVEL(gamestate->version));

	game_run(&gamestate); /* Run the game */

	game_exit(gamestate);
	script_free_engine(gamestate); /* Uninitialize game state */
	script_free_breakpoints(gamestate);
	free(gamestate->work_dir);
	free(gamestate);

	scir_free_resource_manager(resmgr);

	close_console_file();

	gfxop_exit(&gfx_state);

	return Common::kNoError;
}

//} // End of namespace Sci
