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

#ifndef SCI_INCLUDE_ENGINE_H
#define SCI_INCLUDE_ENGINE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/serializer.h"

namespace Common {
	class SeekableReadStream;
	class WriteStream;
}

#include "sci/scicore/vocabulary.h"
#include "sci/scicore/resource.h"
#include "sci/scicore/sciconsole.h"
#include "sci/scicore/versions.h"
#include "sci/engine/kernel.h"	// for kfunct_sig_pair_t
#include "sci/engine/script.h"
#include "sci/engine/seg_manager.h"
#include "sci/gfx/gfx_system.h"
#include "sci/sfx/core.h"

namespace Sci {

class Menubar;
struct kfunct_sig_pair_t;	// from kernel.h

struct gfx_state_t;
struct GfxPort;
struct GfxVisual;
struct GfxContainer;	
struct GfxList;


class DirSeeker {
protected:
	EngineState *_vm;
	reg_t _outbuffer;
	Common::StringList _savefiles;
	Common::StringList::const_iterator _iter;

public:
	DirSeeker(EngineState *s) : _vm(s) {
		_outbuffer = NULL_REG;
		_iter = _savefiles.begin();
	}
	
	void firstFile(const char *mask, reg_t buffer);
	void nextFile();
};

#define CURRENT_SAVEGAME_VERSION 8
#define MINIMUM_SAVEGAME_VERSION 8

#define MAX_SAVEGAME_NR 20 /* Maximum number of savegames */

#define MAX_SAVE_DIR_SIZE MAXPATHLEN

/* values for EngineState.restarting_flag */
#define SCI_GAME_IS_NOT_RESTARTING 0
#define SCI_GAME_WAS_RESTARTED 1
#define SCI_GAME_IS_RESTARTING_NOW 2
#define SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE 4

struct drawn_pic_t {
	int nr;
	int palette;
};

class FileHandle {
public:
	Common::String _name;
	Common::SeekableReadStream *_in;
	Common::WriteStream *_out;
	
public:
	FileHandle();
	~FileHandle();
	
	void close();
	bool isOpen() const;
};

struct EngineState : public Common::Serializable {
public:
	EngineState();
	virtual ~EngineState();
	virtual void saveLoadWithSerializer(Common::Serializer &ser);

public:
	int savegame_version;

	int widget_serial_counter; /* Used for savegames */

	ResourceManager *resmgr; /* The resource manager */

	Common::String _gameName; /* Designation of the primary object (which inherits from Game) */
	char *game_version;

	/* Non-VM information */

	gfx_state_t *gfx_state; /* Graphics state and driver */
	gfx_pixmap_t *old_screen; /* Old screen content: Stored during kDrawPic() for kAnimate() */

	sfx_state_t sound; /* sound subsystem */
	int sfx_init_flags; /* flags the sfx subsystem was initialised with */
	unsigned int sound_volume; /* 0x0 -> 0xf Current volume of sound system */
	unsigned int sound_mute; /* 0 = not, else == saved value */

	byte restarting_flags; /* Flags used for restarting */
	byte have_mouse_flag;  /* Do we have a hardware pointing device? */

	byte pic_not_valid; /* Is 0 if the background picture is "valid" */
	byte pic_is_new; /* New pic was loaded or port was opened */
	byte onscreen_console;  /* Use the onscreen console for debugging */
	byte *osc_backup; /* Backup of the pre-onscreen console screen data */

	int *pic_priority_table; /* 16 entries with priorities or NULL if not present */

	/** Text on the status bar, or NULL if the title bar is blank */
	Common::String _statusBarText;

	int status_bar_foreground, status_bar_background;

	long game_time; /* Counted at 60 ticks per second, reset during start time */

	reg_t save_dir_copy; /* Last copy of the save dir */
	int save_dir_edit_offset; /* For kEdit(): Display offset for editing the savedir */
	char *save_dir_copy_buf; /* Temp savedir buffer for kEdit() */

	int mouse_pointer_view; /* Mouse pointer resource, or -1 if disabled */
	int mouse_pointer_loop; /* Mouse pointer resource, or -1 if disabled */
	int mouse_pointer_cel; /* Mouse pointer resource, or -1 if disabled */
	int save_mouse_pointer_view; /* Temporary storage for mouse pointer resource, when the pointer is hidden */
	int save_mouse_pointer_loop; /* Temporary storage for mouse pointer resource, when the pointer is hidden */
	int save_mouse_pointer_cel; /* Temporary storage for mouse pointer resource, when the pointer is hidden */

	int port_serial; /* Port serial number, for save/restore */
	GfxPort *port; /* The currently active port */

	gfx_color_t ega_colors[16]; /* The 16 EGA colors- for SCI0(1) */

	GfxVisual *visual; /* A visual widget, containing all ports */

	GfxPort *titlebar_port; /* Title bar viewport (0,0,9,319) */
	GfxPort *wm_port; /* window manager viewport and designated &heap[0] view (10,0,199,319) */
	GfxPort *picture_port; /* The background picture viewport (10,0,199,319) */
	GfxPort *iconbar_port; /* Full-screen port used for non-clipped icon bar draw in SCI1 */

	gfx_map_mask_t pic_visible_map; /* The number of the map to display in update commands */
	int pic_animate; /* The animation used by Animate() to display the picture */

	int dyn_views_list_serial; /* Used for save/restore */
	GfxList *dyn_views; /* Pointers to pic and dynamic view lists */

	int drop_views_list_serial; /* Used for save/restore */
	GfxList *drop_views; /* A list Animate() can dump dropped dynviews into */

	long animation_delay; /* A delay factor for pic opening animations. Defaults to 500. */
	int animation_granularity; /* Number of animation steps to perform betwen updates for transition animations */

	Menubar *_menubar; /* The menu bar */

	int priority_first; /* The line where priority zone 0 ends */
	int priority_last; /* The line where the highest priority zone starts */

	int pics_drawn_nr;
	int pics_nr;
	drawn_pic_t *pics;

	uint32 game_start_time; /* The time at which the interpreter was started */
	uint32 last_wait_time; /* The last time the game invoked Wait() */

	byte version_lock_flag; /* Set to 1 to disable any autodetection mechanisms */
	sci_version_t version; /* The approximated patchlevel of the version to emulate */
	sci_version_t max_version, min_version; /* Used for autodetect sanity checks */

	unsigned int kernel_opt_flags; /* Kernel optimization flags- used for performance tweaking */

	/* Kernel File IO stuff */

	Common::Array<FileHandle> _fileHandles; /* Array of file handles. Dynamically increased if required. */

	DirSeeker _dirseeker;

	/* VM Information */

	ExecStack *execution_stack; /* The execution stack */
	int execution_stack_size;      /* Number of stack frames allocated */
	int execution_stack_pos;       /* Position on the execution stack */
	int execution_stack_base;      /* When called from kernel functions, the vm
				       ** is re-started recursively on the same stack.
				       ** This variable contains the stack base for the
				       ** current vm.
				       */
	int execution_stack_pos_changed;   /* Set to 1 if the execution stack position
					   ** should be re-evaluated by the vm
					   */

	reg_t r_acc; /* Accumulator */
	unsigned int r_amp_rest; /* &rest register (only used for save games) */
	reg_t r_prev; /* previous comparison result */

	SegmentId stack_segment; /* Heap area for the stack to use */
	StackPtr stack_base; /* Pointer to the least stack element */
	StackPtr stack_top; /* First invalid stack element */

	SegmentId parser_segment;  /* A heap area used by the parser for error reporting */
	reg_t parser_base; /* Base address for the parser error reporting mechanism */
	reg_t parser_event; /* The event passed to Parse() and later used by Said() */
	SegmentId script_000_segment;
	Script *script_000;  /* script 000, e.g. for globals */

	int parser_lastmatch_word; /* Position of the input word the parser last matched on, or SAID_NO_MATCH */

	/* Debugger data: */
	Breakpoint *bp_list;   /* List of breakpoints */
	int have_bp;  /* Bit mask specifying which types of breakpoints are used in bp_list */
	unsigned int debug_mode; /* Contains flags for the various debug modes */

	/* System strings */
	SegmentId sys_strings_segment;
	SystemStrings *sys_strings;

	SegmentId string_frag_segment;

	/* Parser data: */
	WordMap _parserWords;
	SuffixList _parserSuffixes;
	Common::Array<parse_tree_branch_t> _parserBranches;
	parse_rule_list_t *parser_rules; /* GNF rules used in the parser algorithm */
	parse_tree_node_t parser_nodes[VOCAB_TREE_NODES]; /* The parse tree */

	int parser_valid; /* If something has been correctly parsed */

	SynonymList _synonyms; /* The list of synonyms */

	reg_t game_obj; /* Pointer to the game object */

	int classtable_size; /* Number of classes in the table- for debugging */
	Class *classtable; /* Table of all classes */

	SegManager *seg_manager;
	int gc_countdown; /* Number of kernel calls until next gc */

	Common::StringList _selectorNames;
	Common::StringList _kernelNames; /* List of kernel names */

	Common::Array<kfunct_sig_pair_t> _kfuncTable; /* Table of kernel functions */

	opcode *opcodes;

	selector_map_t selector_map; /* Shortcut list for important selectors */

	EngineState *successor; /* Successor of this state: Used for restoring */
};

PaletteEntry get_pic_color(EngineState *s, int color);
/* Retrieves the gfx_pixmap_color_t associated with a game color index
** Parameters: (EngineState *) s: The game state
**             (int) color: The color to look up
** Returns   : (gfx_pixmap_color_t *) The requested color.
*/

static inline reg_t not_register(EngineState *s, reg_t r) {
	if (s->version >= SCI_VERSION_FTU_INVERSE_CANBEHERE)
		return make_reg(0, !r.offset);
	else
		return r;
}

} // End of namespace Sci

#endif // SCI_INCLUDE_ENGINE_H
