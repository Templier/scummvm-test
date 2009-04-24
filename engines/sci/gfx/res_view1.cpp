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

// SCI 1 view resource defrobnicator

#include "common/endian.h"

#include "sci/sci_memory.h"
#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

#define V1_LOOPS_NR_OFFSET 0
#define V1_MIRROR_MASK 2
#define V1_PALETTE_OFFSET 6
#define V1_FIRST_LOOP_OFFSET 8

#define V1_RLE 0x80 // run-length encode?
#define V1_RLE_BG 0x40 // background fill

#define V2_HEADER_SIZE 0
#define V2_LOOPS_NUM 2
#define V2_PALETTE_OFFSET 8
#define V2_BYTES_PER_LOOP 12
#define V2_BYTES_PER_CEL 13

#define V2_IS_MIRROR 1
#define V2_COPY_OF_LOOP 2
#define V2_CELS_NUM 4
#define V2_LOOP_OFFSET 14

#define NEXT_RUNLENGTH_BYTE(n) \
	if (literal_pos == runlength_pos) \
		literal_pos += n; \
	runlength_pos += n;

#define NEXT_LITERAL_BYTE(n) \
	if (literal_pos == runlength_pos) \
		runlength_pos += n; \
	literal_pos += n;

static int decompress_sci_view(int id, int loop, int cel, byte *resource, byte *dest, int mirrored, int pixmap_size, int size,
	int runlength_pos, int literal_pos, int xl, int yl, int color_key) {
	int writepos = mirrored ? xl : 0;
	int linebase = 0;

	// For some cels the RLE data ends at the last non-transparent pixel,
	// so we initialize the whole pixmap to transparency first
	memset(dest, color_key, pixmap_size);

	while ((mirrored ? linebase < pixmap_size : writepos < pixmap_size) && literal_pos < size && runlength_pos < size) {
		int op = resource[runlength_pos];
		int bytes;
		int readbytes = 0;
		int color = 0;

		NEXT_RUNLENGTH_BYTE(1);

		if (op & V1_RLE) {
			bytes = op & 0x3f;
			op &= (V1_RLE | V1_RLE_BG);
			readbytes = (op & V1_RLE_BG) ? 0 : 1;
		} else {
			readbytes = bytes = op & 0x3f;
			op = 0;
		}

		assert(runlength_pos + readbytes <= size);

		/*
		if (writepos - bytes < 0) {
			GFXWARN("View %02x:(%d/%d) describes more bytes than needed: %d/%d bytes at rel. offset 0x%04x\n",
					id, loop, cel, writepos - bytes, pixmap_size, pos - 1);
			bytes = pixmap_size - writepos;
		}
		*/

		if (mirrored && op == V1_RLE) {
			color = resource[literal_pos];
			NEXT_LITERAL_BYTE(1);
		}

		assert(op || literal_pos + bytes <= size);

		if (!mirrored && (writepos + bytes > pixmap_size)) {
			GFXWARN("Writing out of bounds: %d bytes at %d > size %d\n", bytes, writepos, pixmap_size);
		}

		if (mirrored) {
			while (bytes--) {
				writepos--;
				if (op) {
					*(dest + writepos) = (op & V1_RLE_BG) ? color_key : color;
				} else {
					*(dest + writepos) = *(resource + literal_pos);
					NEXT_LITERAL_BYTE(1);
				}
				if (writepos == linebase) {
					writepos += 2 * xl;
					linebase += xl;
				}
			}
		} else {
			if (op) {
				if (op & V1_RLE_BG)
					memset(dest + writepos, color_key, bytes);
				else {
					color = resource[literal_pos];

					NEXT_LITERAL_BYTE(1);
					memset(dest + writepos, color, bytes);
				}
			} else {
				memcpy(dest + writepos, resource + literal_pos, bytes);
				NEXT_LITERAL_BYTE(bytes);
			}
			writepos += bytes;
		}
	}

	return 0;
}

static int decompress_sci_view_amiga(int id, int loop, int cel, byte *resource, byte *dest, int mirrored, int pixmap_size, int size,
	int pos, int xl, int yl, int color_key) {
	int writepos = mirrored ? xl - 1 : 0;

	while (writepos < pixmap_size && pos < size) {
		int op = resource[pos++];
		int bytes;
		int color;

		if (op & 0x07) {
			bytes = op & 0x07;
			color = op >> 3;
		} else {
			bytes = op >> 3;
			color = color_key;
		}

		if (mirrored) {
			while (bytes--) {
				dest[writepos--] = color;
				// If we've just written the first pixel of a line...
				if (!((writepos + 1) % xl)) {
					// Then move to the end of next line
					writepos += 2 * xl;

					if (writepos >= pixmap_size && bytes) {
						GFXWARN("View %02x:(%d/%d) writing out of bounds\n", id, loop, cel);
						break;
					}
				}
			}
		} else {
			if (writepos + bytes > pixmap_size) {
				GFXWARN("View %02x:(%d/%d) describes more bytes than needed: %d/%d bytes at rel. offset 0x%04x\n",
				        id, loop, cel, writepos - bytes, pixmap_size, pos - 1);
				bytes = pixmap_size - writepos;
			}
			memset(dest + writepos, color, bytes);
			writepos += bytes;
		}
	}

	if (writepos < pixmap_size) {
		GFXWARN("View %02x:(%d/%d) not enough pixel data in view\n", id, loop, cel);
		return 1;
	}

	return 0;
}

gfx_pixmap_t *gfxr_draw_cel1(int id, int loop, int cel, int mirrored, byte *resource, byte *cel_base, int size, gfxr_view_t *view, bool isAmiga, bool isSci11) {
	int xl = READ_LE_UINT16(cel_base);
	int yl = READ_LE_UINT16(cel_base + 2);
	int pixmap_size = xl * yl;
	int xdisplace = isSci11 ? READ_LE_UINT16(cel_base + 4) : (int8) cel_base[4];
	int ydisplace = isSci11 ? READ_LE_UINT16(cel_base + 6) : (uint8) cel_base[5];
	int runlength_offset = isSci11 ? READ_LE_UINT16(cel_base + 24) : 8;
	int literal_offset = isSci11 ? READ_LE_UINT16(cel_base + 28) : 8;
	gfx_pixmap_t *retval = gfx_pixmap_alloc_index_data(gfx_new_pixmap(xl, yl, id, loop, cel));
	byte *dest = retval->index_data;
	int decompress_failed;

	retval->color_key = cel_base[isSci11 ? 8 : 6];
	retval->xoffset = (mirrored) ? xdisplace : -xdisplace;
	retval->yoffset = -ydisplace;
	// FIXME: In LSL5, it seems that the inventory has views without palettes (or we don't load palettes properly)
	retval->palette = (view && view->palette) ? view->palette->getref() : NULL;

	if (xl <= 0 || yl <= 0) {
		gfx_free_pixmap(retval);
		GFXERROR("View %02x:(%d/%d) has invalid xl=%d or yl=%d\n", id, loop, cel, xl, yl);
		return NULL;
	}

	if (!isAmiga)
		decompress_failed = decompress_sci_view(id, loop, cel, resource, dest, mirrored, pixmap_size, size, runlength_offset,
		                                        literal_offset, xl, yl, retval->color_key);
	else
		decompress_failed = decompress_sci_view_amiga(id, loop, cel, resource, dest, mirrored, pixmap_size, size, runlength_offset,
		                    xl, yl, retval->color_key);

	if (decompress_failed) {
		gfx_free_pixmap(retval);
		return NULL;
	}

	return retval;
}

static int gfxr_draw_loop1(gfxr_loop_t *dest, int id, int loop, int mirrored, byte *resource, int offset, int size, gfxr_view_t *view, int amiga_game) {
	int i;
	int cels_nr = READ_LE_UINT16(resource + offset);

	if (READ_LE_UINT16(resource + offset + 2)) {
		GFXWARN("View %02x:(%d): Gray magic %04x in loop, expected white\n", id, loop, READ_LE_UINT16(resource + offset + 2));
	}

	if (cels_nr * 2 + 4 + offset > size) {
		GFXERROR("View %02x:(%d): Offset array for %d cels extends beyond resource space\n", id, loop, cels_nr);
		dest->cels_nr = 0; // Mark as "delete no cels"
		dest->cels = NULL;
		return 1;
	}

	dest->cels = (gfx_pixmap_t**)sci_malloc(sizeof(gfx_pixmap_t *) * cels_nr);

	for (i = 0; i < cels_nr; i++) {
		int cel_offset = READ_LE_UINT16(resource + offset + 4 + (i << 1));
		gfx_pixmap_t *cel;

		if (cel_offset >= size) {
			GFXERROR("View %02x:(%d/%d) supposed to be at illegal offset 0x%04x\n", id, loop, i, cel_offset);
			cel = NULL;
		} else
			cel = gfxr_draw_cel1(id, loop, i, mirrored, resource + cel_offset, resource + cel_offset, size - cel_offset, view, amiga_game, false);

		if (!cel) {
			dest->cels_nr = i;
			return 1;
		}

		dest->cels[i] = cel;
	}

	dest->cels_nr = cels_nr;

	return 0;
}

gfxr_view_t *gfxr_draw_view1(int id, byte *resource, int size, Palette *static_pal) {
	int i;
	int palette_offset;
	gfxr_view_t *view;
	int mirror_mask;
	int amiga_game = 0;

	if (size < V1_FIRST_LOOP_OFFSET + 8) {
		GFXERROR("Attempt to draw empty view %04x\n", id);
		return NULL;
	}

	view = (gfxr_view_t*)sci_malloc(sizeof(gfxr_view_t));
	view->ID = id;
	view->flags = 0;

	view->loops_nr = resource[V1_LOOPS_NR_OFFSET];
	palette_offset = READ_LE_UINT16(resource + V1_PALETTE_OFFSET);
	mirror_mask = READ_LE_UINT16(resource + V1_MIRROR_MASK);

	if (view->loops_nr * 2 + V1_FIRST_LOOP_OFFSET > size) {
		GFXERROR("View %04x: Not enough space in resource to accomodate for the claimed %d loops\n", id, view->loops_nr);
		free(view);
		return NULL;
	}

	if (palette_offset > 0) {
		if (palette_offset > size) {
			GFXERROR("Palette is outside of view %04x\n", id);
			free(view);
			return NULL;
		}
		if (!(view->palette = gfxr_read_pal1(id, resource + palette_offset, size - palette_offset))) {
			GFXERROR("view %04x: Palette reading failed. Aborting...\n", id);
			free(view);
			return NULL;
		}
	} else if (static_pal && static_pal->size() == GFX_SCI1_AMIGA_COLORS_NR) {
		// Assume we're running an amiga game.
		amiga_game = 1;
		view->palette = static_pal->getref();
	} else {
		GFXWARN("view %04x: Doesn't have a palette. Can SCI handle this?\n", view->ID);
		view->palette = NULL;
	}

	view->loops = (gfxr_loop_t*)sci_malloc(sizeof(gfxr_loop_t) * view->loops_nr);

	for (i = 0; i < view->loops_nr; i++) {
		int error_token = 0;
		int loop_offset = READ_LE_UINT16(resource + V1_FIRST_LOOP_OFFSET + (i << 1));

		if (loop_offset >= size) {
			GFXERROR("View %04x:(%d) supposed to be at illegal offset 0x%04x\n", id, i, loop_offset);
			error_token = 1;
		}

		if (error_token || gfxr_draw_loop1(view->loops + i, id, i, mirror_mask & (1 << i), resource, loop_offset, size, view, amiga_game)) {
			// An error occured
			view->loops_nr = i;
			gfxr_free_view(view);
			return NULL;
		}
	}

	return view;
}

gfxr_loop_t *gfxr_draw_loop11(int id, int loop, int mirrored, byte *resource_base, byte *loop_base, int size, int cels_nr,
	gfxr_loop_t *result, gfxr_view_t *view, int bytes_per_cel) {
	byte *seeker = loop_base;
	int i;

	result->cels_nr = cels_nr;
	result->cels = (gfx_pixmap_t **)sci_malloc(sizeof(gfx_pixmap_t *) * cels_nr);

	for (i = 0; i < cels_nr; i++) {
		result->cels[i] = gfxr_draw_cel1(id, loop, i, mirrored, resource_base, seeker, size, view, 0, true);
		seeker += bytes_per_cel;
	}

	return result;
}

gfxr_view_t *gfxr_draw_view11(int id, byte *resource, int size) {
	gfxr_view_t *view;
	int header_size = READ_LE_UINT16(resource + V2_HEADER_SIZE);
	int palette_offset = READ_LE_UINT16(resource + V2_PALETTE_OFFSET);
	int bytes_per_loop = resource[V2_BYTES_PER_LOOP];
	int loops_num = resource[V2_LOOPS_NUM];
	int bytes_per_cel = resource[V2_BYTES_PER_CEL];
	int i;
	byte *seeker;

	view = (gfxr_view_t *)sci_malloc(sizeof(gfxr_view_t));

	memset(view, 0, sizeof(gfxr_view_t));
	view->ID = id;
	view->flags = 0;

	view->loops_nr = loops_num;
	view->loops = (gfxr_loop_t *)calloc(view->loops_nr, sizeof(gfxr_loop_t));

	// There is no indication of size here, but this is certainly large enough
	view->palette = gfxr_read_pal11(id, resource + palette_offset, 1284);

	seeker = resource + header_size;
	for (i = 0; i < view->loops_nr; i++) {
		int loop_offset = READ_LE_UINT16(seeker + V2_LOOP_OFFSET);
		int cels = seeker[V2_CELS_NUM];
		int mirrored = seeker[V2_IS_MIRROR];
		int copy_entry = seeker[V2_COPY_OF_LOOP];

		printf("%d\n", mirrored);
		if (copy_entry == 255)
			gfxr_draw_loop11(id, i, 0, resource, resource + loop_offset, size, cels, view->loops + i,
			                 view, bytes_per_cel);
		else {
			byte *temp = resource + header_size + copy_entry * bytes_per_loop;
			loop_offset = READ_LE_UINT16(temp + V2_LOOP_OFFSET);
			cels = temp[V2_CELS_NUM];
			gfxr_draw_loop11(id, i, 1, resource, resource + loop_offset, size, cels,
			                 view->loops + i, view, bytes_per_cel);
		}

		seeker += bytes_per_loop;
	}

	return view;
}

} // End of namespace Sci
