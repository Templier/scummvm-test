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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "art_svp_vpath.h"
#include "art_svp_vpath_stroke.h"
#include "art_svp_render_aa.h"

#include "vectorimage.h"

namespace Sword25 {

void art_rgb_fill_run1(art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, int n) {
	int i;

	if (r == g && g == b && r == 255) {
		memset(buf, g, n + n + n + n);
	} else {
		uint32 *alt = (uint32 *)buf;
		//uint32 color = Graphics::ARGBToColor<Graphics::ColorMasks<8888> >(0xff, r, g, b);
		uint32 color = BS_ARGB(0xff, r, g, b);

		for (i = 0; i < n; i++)
			*alt++ = color;
	}
}

void art_rgb_run_alpha1(art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, int alpha, int n) {
	int i;
	int v;

	for (i = 0; i < n; i++) {
		v = *buf;
		*buf++ = v + (((b - v) * alpha + 0x80) >> 8);
		v = *buf;
		*buf++ = v + (((g - v) * alpha + 0x80) >> 8);
		v = *buf;
		*buf++ = v + (((r - v) * alpha + 0x80) >> 8);
		v = *buf;
		if (v + alpha > 255)
			*buf++ = 255;
		else
			*buf++ = v + alpha; //(((alpha - v) * alpha + 0x80) >> 8);
	}
}

typedef struct _ArtRgbSVPAlphaData ArtRgbSVPAlphaData;

struct _ArtRgbSVPAlphaData {
	int alphatab[256];
	art_u8 r, g, b, alpha;
	art_u8 *buf;
	int rowstride;
	int x0, x1;
};

static void art_rgb_svp_alpha_callback1(void *callback_data, int y,
                                        int start, ArtSVPRenderAAStep *steps, int n_steps) {
	ArtRgbSVPAlphaData *data = (ArtRgbSVPAlphaData *)callback_data;
	art_u8 *linebuf;
	int run_x0, run_x1;
	art_u32 running_sum = start;
	int x0, x1;
	int k;
	art_u8 r, g, b;
	int *alphatab;
	int alpha;

	linebuf = data->buf;
	x0 = data->x0;
	x1 = data->x1;

	r = data->r;
	g = data->g;
	b = data->b;
	alphatab = data->alphatab;

	if (n_steps > 0) {
		run_x1 = steps[0].x;
		if (run_x1 > x0) {
			alpha = (running_sum >> 16) & 0xff;
			if (alpha)
				art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], run_x1 - x0);
		}

		for (k = 0; k < n_steps - 1; k++) {
			running_sum += steps[k].delta;
			run_x0 = run_x1;
			run_x1 = steps[k + 1].x;
			if (run_x1 > run_x0) {
				alpha = (running_sum >> 16) & 0xff;
				if (alpha)
					art_rgb_run_alpha1(linebuf + (run_x0 - x0) * 4, r, g, b, alphatab[alpha], run_x1 - run_x0);
			}
		}
		running_sum += steps[k].delta;
		if (x1 > run_x1) {
			alpha = (running_sum >> 16) & 0xff;
			if (alpha)
				art_rgb_run_alpha1(linebuf + (run_x1 - x0) * 4, r, g, b, alphatab[alpha], x1 - run_x1);
		}
	} else {
		alpha = (running_sum >> 16) & 0xff;
		if (alpha)
			art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], x1 - x0);
	}

	data->buf += data->rowstride;
}

static void art_rgb_svp_alpha_opaque_callback1(void *callback_data, int y,
        int start,
        ArtSVPRenderAAStep *steps, int n_steps) {
	ArtRgbSVPAlphaData *data = (ArtRgbSVPAlphaData *)callback_data;
	art_u8 *linebuf;
	int run_x0, run_x1;
	art_u32 running_sum = start;
	int x0, x1;
	int k;
	art_u8 r, g, b;
	int *alphatab;
	int alpha;

	linebuf = data->buf;
	x0 = data->x0;
	x1 = data->x1;

	r = data->r;
	g = data->g;
	b = data->b;
	alphatab = data->alphatab;

	if (n_steps > 0) {
		run_x1 = steps[0].x;
		if (run_x1 > x0) {
			alpha = running_sum >> 16;
			if (alpha) {
				if (alpha >= 255)
					art_rgb_fill_run1(linebuf, r, g, b, run_x1 - x0);
				else
					art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], run_x1 - x0);
			}
		}

		for (k = 0; k < n_steps - 1; k++) {
			running_sum += steps[k].delta;
			run_x0 = run_x1;
			run_x1 = steps[k + 1].x;
			if (run_x1 > run_x0) {
				alpha = running_sum >> 16;
				if (alpha) {
					if (alpha >= 255)
						art_rgb_fill_run1(linebuf + (run_x0 - x0) * 4, r, g, b, run_x1 - run_x0);
					else
						art_rgb_run_alpha1(linebuf + (run_x0 - x0) * 4, r, g, b, alphatab[alpha], run_x1 - run_x0);
				}
			}
		}
		running_sum += steps[k].delta;
		if (x1 > run_x1) {
			alpha = running_sum >> 16;
			if (alpha) {
				if (alpha >= 255)
					art_rgb_fill_run1(linebuf + (run_x1 - x0) * 4, r, g, b, x1 - run_x1);
				else
					art_rgb_run_alpha1(linebuf + (run_x1 - x0) * 4, r, g, b, alphatab[alpha], x1 - run_x1);
			}
		}
	} else {
		alpha = running_sum >> 16;
		if (alpha) {
			if (alpha >= 255)
				art_rgb_fill_run1(linebuf, r, g, b, x1 - x0);
			else
				art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], x1 - x0);
		}
	}

	data->buf += data->rowstride;
}

void art_rgb_svp_alpha1(const ArtSVP *svp,
                        int x0, int y0, int x1, int y1,
                        uint32 color,
                        art_u8 *buf, int rowstride) {
	ArtRgbSVPAlphaData data;
	byte r, g, b, alpha;
	int i;
	int a, da;

	//Graphics::colorToARGB<Graphics::ColorMasks<8888> >(color, alpha, r, g, b);
	alpha = (color >> 24) & 0xff;
	b = (color >> 16) & 0xff;
	g = (color >> 8) & 0xff;
	r = (color >> 0) & 0xff;

	data.r = r;
	data.g = g;
	data.b = b;
	data.alpha = alpha;

	a = 0x8000;
	da = (alpha * 66051 + 0x80) >> 8; /* 66051 equals 2 ^ 32 / (255 * 255) */

	for (i = 0; i < 256; i++) {
		data.alphatab[i] = a >> 16;
		a += da;
	}

	data.buf = buf;
	data.rowstride = rowstride;
	data.x0 = x0;
	data.x1 = x1;
	if (alpha == 255)
		art_svp_render_aa(svp, x0, y0, x1, y1, art_rgb_svp_alpha_opaque_callback1, &data);
	else
		art_svp_render_aa(svp, x0, y0, x1, y1, art_rgb_svp_alpha_callback1, &data);
}

static int art_vpath_len(ArtVpath *a) {
	int i;

	for (i = 0; a[i].code != ART_END; i++);
	return i;
}

ArtVpath *art_vpath_cat(ArtVpath *a, ArtVpath *b) {
	ArtVpath *dest;
	ArtVpath *p;
	int len_a, len_b;

	len_a = art_vpath_len(a);
	len_b = art_vpath_len(b);
	dest = art_new(ArtVpath, len_a + len_b + 1);
	p = dest;

	for (int i = 0; i < len_a; i++)
		*p++ = *a++;
	for (int i = 0; i <= len_b; i++)
		*p++ = *b++;

	return dest;
}

void art_svp_make_convex(ArtSVP *svp) {
	int i;

	if (svp->n_segs > 0 && svp->segs[0].dir == 0) {
		for (i = 0; i < svp->n_segs; i++) {
			svp->segs[i].dir = !svp->segs[i].dir;
		}
	}
}

ArtVpath *art_vpath_reverse(ArtVpath *a) {
	ArtVpath *dest;
	ArtVpath it;
	int len;
	int state = 0;
	int i;

	len = art_vpath_len(a);
	dest = art_new(ArtVpath, len + 1);

	for (i = 0; i < len; i++) {
		it = a[len - i - 1];
		if (state) {
			it.code = ART_LINETO;
		} else {
			it.code = ART_MOVETO_OPEN;
			state = 1;
		}
		if (a[len - i - 1].code == ART_MOVETO ||
		        a[len - i - 1].code == ART_MOVETO_OPEN) {
			state = 0;
		}
		dest[i] = it;
	}
	dest[len] = a[len];

	return dest;
}

ArtVpath *art_vpath_reverse_free(ArtVpath *a) {
	ArtVpath *dest;

	dest = art_vpath_reverse(a);
	free(a);

	return dest;
}

void drawBez(ArtBpath *bez1, ArtBpath *bez2, art_u8 *buffer, int width, int height, int deltaX, int deltaY, double scaleX, double scaleY, double penWidth, unsigned int color) {
	ArtVpath *vec = NULL;
	ArtVpath *vec1 = NULL;
	ArtVpath *vec2 = NULL;
	ArtSVP *svp = NULL;

#if DEBUG
	const char *codes[] = {"ART_MOVETO", "ART_MOVETO_OPEN", "ART_CURVETO", "ART_LINETO", "ART_END"};
	for (int i = 0;; i++) {
		printf("    bez1[%d].code = %s;\n", i, codes[bez1[i].code]);
		if (bez1[i].code == ART_END)
			break;
		if (bez1[i].code == ART_CURVETO) {
			printf("    bez1[%d].x1 = %f; bez1[%d].y1 = %f;\n", i, bez1[i].x1, i, bez1[i].y1);
			printf("    bez1[%d].x2 = %f; bez1[%d].y2 = %f;\n", i, bez1[i].x2, i, bez1[i].y2);
		}
		printf("    bez1[%d].x3 = %f; bez1[%d].y3 = %f;\n", i, bez1[i].x3, i, bez1[i].y3);
	}
	if (bez2 != 0) {
		for (int i = 0;; i++) {
			printf("    bez2[%d].code = %s;\n", i, codes[bez2[i].code]);
			if (bez2[i].code == ART_END)
				break;
			if (bez2[i].code == ART_CURVETO) {
				printf("    bez2[%d].x1 = %f; bez2[%d].y1 = %f;\n", i, bez2[i].x1, i, bez2[i].y1);
				printf("    bez2[%d].x2 = %f; bez2[%d].y2 = %f;\n", i, bez2[i].x2, i, bez2[i].y2);
			}
			printf("    bez2[%d].x3 = %f; bez2[%d].y3 = %f;\n", i, bez2[i].x3, i, bez2[i].y3);
		}
		printf("    drawBez(bez1, bez2, buffer, 1.0, 1.0, %f, 0x%08x);\n", penWidth, color);
	} else {
		printf("    drawBez(bez1, 0, buffer, 1.0, 1.0, %f, 0x%08x);\n", penWidth, color);
	}

#endif

	vec1 = art_bez_path_to_vec(bez1, 0.5);
	if (bez2 != 0) {
		vec2 = art_bez_path_to_vec(bez2, 0.5);
		vec2 = art_vpath_reverse_free(vec2);
		vec = art_vpath_cat(vec1, vec2);

		free(vec1);
		free(vec2);
	} else {
		vec = vec1;
	}

	if (scaleX != 1.0 || scaleY != 1.0) {
		ArtVpath *vect;
		int size = art_vpath_len(vec);

		vect = art_new(ArtVpath, size + 1);

		int k;
		for (k = 0; k < size; k++) {
			vect[k].code = vec[k].code;
			vect[k].x = vec[k].x * scaleX;
			vect[k].y = vec[k].y * scaleY;
		}
		vect[k].code = ART_END;
		free(vec);

		vec = vect;
	}

	if (bez2 == 0) { // Line drawing
		svp = art_svp_vpath_stroke(vec, ART_PATH_STROKE_JOIN_ROUND, ART_PATH_STROKE_CAP_ROUND, penWidth, 1.0, 0.5);
	} else {
		svp = art_svp_from_vpath(vec);
		art_svp_make_convex(svp);
	}

	art_rgb_svp_alpha1(svp, 0 + deltaX, 0 + deltaY, width + deltaX, height + deltaY, color, buffer, width * 4);

	free(svp);
	free(vec);
}

void VectorImage::render(int width, int height) {
	double scaleX = (width == - 1) ? 1 : static_cast<double>(width) / static_cast<double>(getWidth());
	double scaleY = (height == - 1) ? 1 : static_cast<double>(height) / static_cast<double>(getHeight());

	//debug(0, "VectorImage::render(%d, %d) %s", width, height, _fname.c_str());

	printf("%d x %d\n", width, height);
	printf("%d x %d - %d x %d\n", _boundingBox.left, _boundingBox.top, _boundingBox.right, _boundingBox.bottom);

	if (_pixelData)
		free(_pixelData);

	_pixelData = (byte *)malloc(width * height * 4);
	memset(_pixelData, 0, width * height * 4);

	for (uint e = 0; e < _elements.size(); e++) {
#if DEBUG
		printf("----------------> Element\n");
#endif

		//// Draw shapes
		for (uint s = 0; s < _elements[e].getFillStyleCount(); s++) {
			int fill0len = 0;
			int fill1len = 0;

			// Count vector sizes in order to minimize memory
			// fragmentation
			for (uint p = 0; p < _elements[e].getPathCount(); p++) {
				if (_elements[e].getPathInfo(p).getFillStyle0() == s + 1)
					fill0len += _elements[e].getPathInfo(p).getVecLen();

				if (_elements[e].getPathInfo(p).getFillStyle1() == s + 1)
					fill1len += _elements[e].getPathInfo(p).getVecLen();
			}

			// Now lump together vectors
			ArtBpath *fill1 = art_new(ArtBpath, fill1len + 1);
			ArtBpath *fill0 = art_new(ArtBpath, fill0len + 1);
			ArtBpath *fill1pos = fill1;
			ArtBpath *fill0pos = fill0;

			for (uint p = 0; p < _elements[e].getPathCount(); p++) {
				if (_elements[e].getPathInfo(p).getFillStyle0() == s + 1) {
					for (int i = 0; i < _elements[e].getPathInfo(p).getVecLen(); i++)
						*fill0pos++ = _elements[e].getPathInfo(p).getVec()[i];
				}

				if (_elements[e].getPathInfo(p).getFillStyle1() == s + 1) {
					for (int i = 0; i < _elements[e].getPathInfo(p).getVecLen(); i++)
						*fill1pos++ = _elements[e].getPathInfo(p).getVec()[i];
				}
			}

			// Close vectors
			(*fill0pos).code = ART_END;
			(*fill1pos).code = ART_END;

			drawBez(fill1, fill0, _pixelData, width, height, _boundingBox.left, _boundingBox.top, scaleX, scaleY, -1, _elements[e].getFillStyleColor(s));

			free(fill0);
			free(fill1);
		}

		//// Draw strokes
		for (uint s = 0; s < _elements[e].getLineStyleCount(); s++) {
			double penWidth = _elements[e].getLineStyleWidth(s);
			penWidth *= sqrt(fabs(scaleX * scaleY));

			for (uint p = 0; p < _elements[e].getPathCount(); p++) {
				if (_elements[e].getPathInfo(p).getLineStyle() == s + 1) {
					drawBez(_elements[e].getPathInfo(p).getVec(), 0, _pixelData, width, height, _boundingBox.left, _boundingBox.top, scaleX, scaleY, penWidth, _elements[e].getLineStyleColor(s));
				}
			}
		}
	}
}


} // End of namespace Sword25
