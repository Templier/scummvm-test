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

#include "common/scummsys.h"
#include "common/system.h"
#include "common/events.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/gfx/gfx_driver.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

struct _scummvm_driver_state {
	gfx_pixmap_t *priority[2];
	byte *visual[2];
	uint8 *pointer_data;
	int xsize, ysize;
};

#define S ((struct _scummvm_driver_state *)(drv->state))

int _modifierStates;

static int scummvm_init_specific(gfx_driver_t *drv, int xfact, int yfact, int bytespp) {
	int i;

	if (!drv->state) // = S
		drv->state = new _scummvm_driver_state;
	if (!drv->state)
		return GFX_FATAL;

	S->xsize = xfact * 320;
	S->ysize = yfact * 200;

	S->pointer_data = NULL;
	//S->buckystate = 0;

	for (i = 0; i < 2; i++) {
		S->priority[i] = gfx_pixmap_alloc_index_data(gfx_new_pixmap(S->xsize, S->ysize, GFX_RESID_NONE, -i, -777));
		if (!S->priority[i]) {
			printf("Out of memory: Could not allocate priority maps! (%dx%d)\n", S->xsize, S->ysize);
			return GFX_FATAL;
		}
	}
	// create the visual buffers
	for (i = 0; i < 2; i++) {
		S->visual[i] = NULL;
		S->visual[i] = new byte[S->xsize * S->ysize];
		if (!S->visual[i]) {
			printf("Out of memory: Could not allocate visual buffers! (%dx%d)\n", S->xsize, S->ysize);
			return GFX_FATAL;
		}
		memset(S->visual[i], 0, S->xsize * S->ysize);
	}

	Graphics::PixelFormat format = { bytespp, 0, 0, 0, 0, 0, 0, 0, 0 };
	drv->mode = gfx_new_mode(xfact, yfact, format, 256, 0);

	return GFX_OK;
}

static int scummvm_init(gfx_driver_t *drv) {
	_modifierStates = 0;
	return scummvm_init_specific(drv, 1, 1, GFX_COLOR_MODE_INDEX);
}

static void scummvm_exit(gfx_driver_t *drv) {
	int i;
	if (S) {
		for (i = 0; i < 2; i++) {
			gfx_free_pixmap(drv, S->priority[i]);
			S->priority[i] = NULL;
		}

		for (i = 0; i < 2; i++) {
			delete[] S->visual[i];
			S->visual[i] = NULL;
		}

		delete[] S->pointer_data;
		S->pointer_data = NULL;

		delete S;
	}
}


// Drawing operations

static void drawProc(int x, int y, int c, void *data) {
	gfx_driver_t *drv = (gfx_driver_t *)data;
	uint8 *p = S->visual[0];
	p[y * 320*drv->mode->xfact + x] = c;
}

static int scummvm_draw_line(gfx_driver_t *drv, Common::Point start, Common::Point end,
	gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	uint32 scolor = color.visual.global_index;
	int xfact = (line_mode == GFX_LINE_MODE_FINE)? 1: drv->mode->xfact;
	int yfact = (line_mode == GFX_LINE_MODE_FINE)? 1: drv->mode->yfact;
	int xsize = S->xsize;
	int ysize = S->ysize;

	if (color.mask & GFX_MASK_VISUAL) {
		Common::Point nstart, nend;

		for (int xc = 0; xc < xfact; xc++) {
			for (int yc = 0; yc < yfact; yc++) {

				nstart.x = CLIP<int16>(start.x + xc, 0, xsize);
				nstart.y = CLIP<int16>(start.y + yc, 0, ysize);
				nend.x = CLIP<int16>(end.x + xc, 0, xsize - 1);
				nend.y = CLIP<int16>(end.y + yc, 0, ysize - 1);

				Graphics::drawLine(nstart.x, nstart.y, nend.x, nend.y, scolor, drawProc, drv);

				if (color.mask & GFX_MASK_PRIORITY) {
					gfx_draw_line_pixmap_i(S->priority[0], nstart, nend, color.priority);
				}
			}
		}
	}

	return GFX_OK;
}

static int scummvm_draw_filled_rect(gfx_driver_t *drv, rect_t rect, gfx_color_t color1, gfx_color_t color2,
	gfx_rectangle_fill_t shade_mode) {
	if (color1.mask & GFX_MASK_VISUAL) {
		for (int i = rect.y; i < rect.y + rect.yl; i++) {
			memset(S->visual[0] + i * S->xsize + rect.x, color1.visual.global_index, rect.xl);
		}
	}

	if (color1.mask & GFX_MASK_PRIORITY)
		gfx_draw_box_pixmap_i(S->priority[0], rect, color1.priority);

	return GFX_OK;
}

// Pixmap operations

static int scummvm_draw_pixmap(gfx_driver_t *drv, gfx_pixmap_t *pxm, int priority,
							   rect_t src, rect_t dest, gfx_buffer_t buffer) {
	int bufnr = (buffer == GFX_BUFFER_STATIC) ? 1 : 0;

	if (dest.xl != src.xl || dest.yl != src.yl) {
		printf("Attempt to scale pixmap (%dx%d)->(%dx%d): Not supported\n", src.xl, src.yl, dest.xl, dest.yl);
		return GFX_ERROR;
	}

	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest, S->visual[bufnr], S->xsize,
	                     S->priority[bufnr]->index_data, S->priority[bufnr]->index_xl, 1, 0);

	return GFX_OK;
}

static int scummvm_grab_pixmap(gfx_driver_t *drv, rect_t src, gfx_pixmap_t *pxm, gfx_map_mask_t map) {
	if (src.x < 0 || src.y < 0) {
		printf("Attempt to grab pixmap from invalid coordinates (%d,%d)\n", src.x, src.y);
		return GFX_ERROR;
	}

	if (!pxm->data) {
		printf("Attempt to grab pixmap to unallocated memory\n");
		return GFX_ERROR;
	}

	switch (map) {

	case GFX_MASK_VISUAL:
		pxm->xl = src.xl;
		pxm->yl = src.yl;
		for (int i = 0; i < src.yl; i++) {
			memcpy(pxm->data + i * src.xl, S->visual[0] + (i + src.y) * S->xsize + src.x, src.xl);
		}
		break;

	case GFX_MASK_PRIORITY:
		printf("FIXME: priority map grab not implemented yet!\n");
		break;

	default:
		printf("Attempt to grab pixmap from invalid map 0x%02x\n", map);
		return GFX_ERROR;
	}

	return GFX_OK;
}

// Buffer operations

static int scummvm_update(gfx_driver_t *drv, rect_t src, Common::Point dest, gfx_buffer_t buffer) {
	//TODO

	/*
	if (src.x != dest.x || src.y != dest.y) {
		printf("Updating %d (%d,%d)(%dx%d) to (%d,%d) on %d\n", buffer, src.x, src.y, src.xl, src.yl, dest.x, dest.y, data_dest);
	} else {
		printf("Updating %d (%d,%d)(%dx%d) to %d\n", buffer, src.x, src.y, src.xl, src.yl, data_dest);
	}
	*/

	switch (buffer) {
	case GFX_BUFFER_BACK:
		for (int i = 0; i < src.yl; i++) {
			memcpy(S->visual[0] + (dest.y + i) * S->xsize + dest.x,
			       S->visual[1] + (src.y + i) * S->xsize + src.x, src.xl);
		}

		if ((src.x == dest.x) && (src.y == dest.y))
			gfx_copy_pixmap_box_i(S->priority[0], S->priority[1], src);
		break;
	case GFX_BUFFER_FRONT:
		g_system->copyRectToScreen(S->visual[0] + src.x + src.y * S->xsize, S->xsize, dest.x, dest.y, src.xl, src.yl);
		g_system->updateScreen();
		break;
	default:
		GFXERROR("Invalid buffer %d in update!\n", buffer);
		return GFX_ERROR;
	}

	return GFX_OK;
}

static int scummvm_set_static_buffer(gfx_driver_t *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority) {
	memcpy(S->visual[1], pic->data, S->xsize * S->ysize);
	gfx_copy_pixmap_box_i(S->priority[1], priority, gfx_rect(0, 0, S->xsize, S->ysize));

	return GFX_OK;
}

// Mouse pointer operations

// Scale cursor and map its colors to the global palette
static uint8 *create_cursor(gfx_driver_t *drv, gfx_pixmap_t *pointer, int mode)
{
	int linewidth = pointer->xl;
	int lines = pointer->yl;
	uint8 *data = new uint8[linewidth*lines];
	uint8 *linebase = data, *pos;
	uint8 *src = pointer->index_data;

	for (int yc = 0; yc < pointer->index_yl; yc++) {
		pos = linebase;

		for (int xc = 0; xc < pointer->index_xl; xc++) {
			uint8 color = *src;
			if (color != 255)
				color = pointer->colors[color].global_index;
			for (int scalectr = 0; scalectr < drv->mode->xfact; scalectr++) {
				*pos++ = color;
			}
			src++;
		}
		for (int scalectr = 1; scalectr < drv->mode->yfact; scalectr++)
			memcpy(linebase + linewidth * scalectr, linebase, linewidth);
		linebase += linewidth * drv->mode->yfact;
	}
	return data;
}


static int scummvm_set_pointer(gfx_driver_t *drv, gfx_pixmap_t *pointer) {
	if (pointer == NULL) {
		g_system->showMouse(false);
	} else {
		delete[] S->pointer_data;
		S->pointer_data = create_cursor(drv, pointer, 1);
		g_system->setMouseCursor(S->pointer_data, pointer->xl, pointer->yl, pointer->xoffset, pointer->yoffset);
		g_system->showMouse(true);
	}

	return GFX_OK;
}

// Palette operations

static int scummvm_set_palette(gfx_driver_t *drv, int index, byte red, byte green, byte blue) {
	if (index < 0 || index > 255) {
		GFXERROR("Attempt to set invalid palette entry %d\n", index);
		return GFX_ERROR;
	}

	byte color[] = {red, green, blue, 255};
	g_system->setPalette(color, index, 1);

	return GFX_OK;
}

// Event management

static sci_event_t scummvm_get_event(gfx_driver_t *drv) {
	sci_event_t input = { SCI_EVT_NONE, 0, 0, 0 };

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	bool found = em->pollEvent(ev);
	Common::Point p = ev.mouse;

	// Don't generate events for mouse movement
	while (found && ev.type == Common::EVENT_MOUSEMOVE) {
		drv->pointer_x = ev.mouse.x;
		drv->pointer_y = ev.mouse.y;
		found = em->pollEvent(ev);
	}

	if (found && !ev.synthetic && ev.type != Common::EVENT_MOUSEMOVE) {
		int modifiers = em->getModifierState();

		// We add the modifier key status to buckybits
		// SDL sends a keydown event if a modifier key is turned on and a keyup event if it's off
		//
 	 	// FIXME: This code is semi-bogus. It only records the modifier key being *pressed*.
 	 	// It does not track correctly whether capslock etc. is active. To do that, we
 	 	// would have to record the fact that the modifier was pressed in global var,
 	 	// and also watch for Common::EVENT_KEYUP events.
 	 	// But this is still not quite good enough, because not all events might
 	 	// pass through here (e.g. the GUI might be running with its own event loop).
 	 	//
 	 	// The best solution likely would be to add code to the EventManager class
 	 	// for tracking which keys are pressed and which are not...
		if (ev.type == Common::EVENT_KEYDOWN || ev.type == Common::EVENT_KEYUP) {
 			switch (ev.kbd.keycode) {
			case Common::KEYCODE_CAPSLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_EVM_CAPSLOCK;
				} else {
					_modifierStates &= ~SCI_EVM_CAPSLOCK;
				}
				break;
			case Common::KEYCODE_NUMLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_EVM_NUMLOCK;
				} else {
					_modifierStates &= ~SCI_EVM_NUMLOCK;
				}
				break;
			case Common::KEYCODE_SCROLLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_EVM_SCRLOCK;
				} else {
					_modifierStates &= ~SCI_EVM_SCRLOCK;
				}
				break;
			default:
				break;
			}
		}
		//TODO: SCI_EVM_INSERT

		input.buckybits =
		    ((modifiers & Common::KBD_ALT) ? SCI_EVM_ALT : 0) |
		    ((modifiers & Common::KBD_CTRL) ? SCI_EVM_CTRL : 0) |
		    ((modifiers & Common::KBD_SHIFT) ? SCI_EVM_LSHIFT | SCI_EVM_RSHIFT : 0) |
			_modifierStates;

		switch (ev.type) {
			// Keyboard events
		case Common::EVENT_KEYDOWN:
			input.data = ev.kbd.keycode;
			input.character = ev.kbd.ascii;

			// Debug console
			if (ev.kbd.flags == Common::KBD_CTRL && ev.kbd.keycode == Common::KEYCODE_d) {

				((Sci::SciEngine*)g_engine)->_console->attach();
				((Sci::SciEngine*)g_engine)->_console->onFrame();

				// Clear keyboard event
				input.type = SCI_EVT_NONE;
				input.character = 0;
				input.data = 0;
				input.buckybits = 0;

				return input;
			}

			if (!(input.data & 0xFF00)) {
				// Directly accept most common keys without conversion
				input.type = SCI_EVT_KEYBOARD;
				if (input.data == Common::KEYCODE_TAB) {
					// Tab
					input.type = SCI_EVT_KEYBOARD;
					input.data = SCI_K_TAB;
					if (input.buckybits & (SCI_EVM_LSHIFT | SCI_EVM_RSHIFT))
						input.character = SCI_K_SHIFT_TAB;
					else
						input.character = SCI_K_TAB;
				}
			} else if ((input.data >= Common::KEYCODE_F1) && input.data <= Common::KEYCODE_F10) {
				// F1-F10
				input.type = SCI_EVT_KEYBOARD;
				// SCI_K_F1 == 59 << 8
				// SCI_K_SHIFT_F1 == 84 << 8
				input.data = SCI_K_F1 + ((input.data - Common::KEYCODE_F1)<<8);
				if (input.buckybits & (SCI_EVM_LSHIFT | SCI_EVM_RSHIFT))
					input.character = input.data + SCI_K_SHIFT_F1 - SCI_K_F1;
				else
					input.character = input.data;
			} else {
				// Special keys that need conversion
				input.type = SCI_EVT_KEYBOARD;
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_UP:
					input.data = SCI_K_UP;
					break;
				case Common::KEYCODE_DOWN:
					input.data = SCI_K_DOWN;
					break;
				case Common::KEYCODE_RIGHT:
					input.data = SCI_K_RIGHT;
					break;
				case Common::KEYCODE_LEFT:
					input.data = SCI_K_LEFT;
					break;
				case Common::KEYCODE_INSERT:
					input.data = SCI_K_INSERT;
					break;
				case Common::KEYCODE_HOME:
					input.data = SCI_K_HOME;
					break;
				case Common::KEYCODE_END:
					input.data = SCI_K_END;
					break;
				case Common::KEYCODE_PAGEUP:
					input.data = SCI_K_PGUP;
					break;
				case Common::KEYCODE_PAGEDOWN:
					input.data = SCI_K_PGDOWN;
					break;
				case Common::KEYCODE_DELETE:
					input.data = SCI_K_DELETE;
					break;
				// Keypad keys
				case Common::KEYCODE_KP8:	// up
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_UP;
					break;
				case Common::KEYCODE_KP2:	// down
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_DOWN;
					break;
				case Common::KEYCODE_KP6:	// right
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_RIGHT;
					break;
				case Common::KEYCODE_KP4:	// left
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_LEFT;
					break;
				case Common::KEYCODE_KP5:	// center
					if (!(_modifierStates & SCI_EVM_NUMLOCK))
						input.data = SCI_K_CENTER;
					break;
				default:
					input.type = SCI_EVT_NONE;
					break;
				}
				input.character = input.data;
			}
			break;

			// Mouse events
		case Common::EVENT_LBUTTONDOWN:
			input.type = SCI_EVT_MOUSE_PRESS;
			input.data = 1;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_RBUTTONDOWN:
			input.type = SCI_EVT_MOUSE_PRESS;
			input.data = 2;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_LBUTTONUP:
			input.type = SCI_EVT_MOUSE_RELEASE;
			input.data = 1;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_RBUTTONUP:
			input.type = SCI_EVT_MOUSE_RELEASE;
			input.data = 2;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;

			// Misc events
		case Common::EVENT_QUIT:
			input.type = SCI_EVT_QUIT;
			break;

		default:
			break;
		}
	}

	return input;
}

gfx_driver_t gfx_driver_scummvm = {
	NULL,
	0, 0,
	0,		// flags here
	0,
	NULL,
	scummvm_init_specific,
	scummvm_init,
	scummvm_exit,
	scummvm_draw_line,
	scummvm_draw_filled_rect,
	scummvm_draw_pixmap,
	scummvm_grab_pixmap,
	scummvm_update,
	scummvm_set_static_buffer,
	scummvm_set_pointer,
	scummvm_set_palette,
	scummvm_get_event,
	NULL
};

} // End of namespace Sci
