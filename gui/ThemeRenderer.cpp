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
 *
 */

#include "common/util.h"
#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "common/system.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "gui/launcher.h"

#include "gui/ThemeRenderer.h"
#include "gui/ThemeEval.h"
#include "graphics/VectorRenderer.h"

namespace GUI {

using namespace Graphics;

const ThemeRenderer::DrawDataInfo ThemeRenderer::kDrawDataDefaults[] = {
	{kDDMainDialogBackground, "mainmenu_bg", true, kDDNone},
	{kDDSpecialColorBackground, "special_bg", true, kDDNone},
	{kDDPlainColorBackground, "plain_bg", true, kDDNone},
	{kDDDefaultBackground, "default_bg", true, kDDNone},
	{kDDTextSelectionBackground, "text_selection", false, kDDNone},

	{kDDWidgetBackgroundDefault, "widget_default", true, kDDNone},
	{kDDWidgetBackgroundSmall, "widget_small", true, kDDNone},
	{kDDWidgetBackgroundEditText, "widget_textedit", true, kDDNone},
	{kDDWidgetBackgroundSlider, "widget_slider", true, kDDNone},

	{kDDButtonIdle, "button_idle", true, kDDNone},
	{kDDButtonHover, "button_hover", false, kDDButtonIdle},
	{kDDButtonDisabled, "button_disabled", true, kDDNone},

	{kDDSliderFull, "slider_full", false, kDDNone},
	{kDDSliderHover, "slider_hover", false, kDDNone},
	{kDDSliderDisabled, "slider_disabled", true, kDDNone},

	{kDDCheckboxDefault, "checkbox_default", true, kDDNone},
	{kDDCheckboxDisabled, "checkbox_disabled", true, kDDNone},
	{kDDCheckboxSelected, "checkbox_selected", false, kDDCheckboxDefault},

	{kDDTabActive, "tab_active", false, kDDTabInactive},
	{kDDTabInactive, "tab_inactive", true, kDDNone},
	{kDDTabBackground, "tab_background", true, kDDNone},

	{kDDScrollbarBase, "scrollbar_base", true, kDDNone},
	
	{kDDScrollbarButtonIdle, "scrollbar_button_idle", true, kDDNone},
	{kDDScrollbarButtonHover, "scrollbar_button_hover", false, kDDScrollbarButtonIdle},
		
	{kDDScrollbarHandleIdle, "scrollbar_handle_idle", false, kDDNone},
	{kDDScrollbarHandleHover, "scrollbar_handle_hover", false, kDDScrollbarBase},

	{kDDPopUpIdle, "popup_idle", true, kDDNone},
	{kDDPopUpHover, "popup_hover", false, kDDPopUpIdle},

	{kDDCaret, "caret", false, kDDNone},
	{kDDSeparator, "separator", true, kDDNone},
};

const ThemeRenderer::TextDataInfo ThemeRenderer::kTextDataDefaults[] = {
	{kTextDataDefault, "text_default"},
	{kTextDataHover, "text_hover"},
	{kTextDataDisabled, "text_disabled"},
	{kTextDataInverted, "text_inverted"},
	{kTextDataButton, "text_button"},
	{kTextDataButtonHover, "text_button_hover"}
};


ThemeRenderer::ThemeRenderer(Common::String themeName, GraphicsMode mode) : 
	_vectorRenderer(0), _system(0), _graphicsMode(kGfxDisabled), 
	_screen(0), _backBuffer(0), _bytesPerPixel(0), _initOk(false), 
	_themeOk(false), _enabled(false), _buffering(false) {
	_system = g_system;
	_parser = new ThemeParser(this);
	_themeEval = new GUI::ThemeEval();

	for (int i = 0; i < kDrawDataMAX; ++i) {
		_widgets[i] = 0;
	}
	
	for (int i = 0; i < kTextDataMAX; ++i) {
		_texts[i] = 0;
	}

	_graphicsMode = mode;
	setGraphicsMode(_graphicsMode);

	loadConfigFile("modern");

	if (_screen->w >= 400 && _screen->h >= 300) {
		_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	} else {
		_font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	}

	_initOk = true;
	_themeName = themeName;
}

bool ThemeRenderer::init() {
	// reset everything and reload the graphics
	deinit();
	setGraphicsMode(_graphicsMode);

	if (_screen->pixels && _backBuffer->pixels) {
		_initOk = true;
		clearAll();
		resetDrawArea();
	}

	if (isThemeLoadingRequired() || !_themeOk) {
		loadTheme(_themeName);

		Theme::loadTheme(_defaultConfig);
		Theme::loadTheme(_configFile, false, true);
	}

	return true;
}

void ThemeRenderer::deinit() {
	if (_initOk) {
		_system->hideOverlay();
		freeRenderer();
		freeScreen();
		freeBackbuffer();
		_initOk = false;
	}
}

void ThemeRenderer::clearAll() {
	if (!_initOk)
		return;

	_system->clearOverlay();
	_system->grabOverlay((OverlayColor*)_screen->pixels, _screen->w);
}

void ThemeRenderer::enable() {
	init();
	resetDrawArea();
	_system->showOverlay();
	clearAll();
	_enabled = true;
}

void ThemeRenderer::disable() {
	_system->hideOverlay();
	_enabled = false;
}

template<typename PixelType> 
void ThemeRenderer::screenInit(bool backBuffer) {
	freeScreen();
	freeBackbuffer();

	uint32 width = _system->getOverlayWidth();
	uint32 height = _system->getOverlayHeight();
	
	if (backBuffer) {
		_backBuffer = new Surface;
		_backBuffer->create(width, height, sizeof(PixelType));
	}
	
	_screen = new Surface;
	_screen->create(width, height, sizeof(PixelType));
	_system->clearOverlay();
}

void ThemeRenderer::setGraphicsMode(GraphicsMode mode) {
	switch (mode) {
	case kGfxStandard16bit:
	case kGfxAntialias16bit:
		_bytesPerPixel = sizeof(uint16);
		screenInit<uint16>(kEnableBackCaching);
		break;

	default:
		return;
	}

	freeRenderer();
	_vectorRenderer = createRenderer(mode);
	_vectorRenderer->setSurface(_screen);
}

void ThemeRenderer::addDrawStep(const Common::String &drawDataId, Graphics::DrawStep step) {
	DrawData id = getDrawDataId(drawDataId);
	
	assert(_widgets[id] != 0);
	_widgets[id]->_steps.push_back(step);
}

bool ThemeRenderer::addTextData(const Common::String &drawDataId, const Common::String &textDataId, TextAlign alignH, TextAlignVertical alignV) {
	DrawData id = getDrawDataId(drawDataId);
	TextData textId = getTextDataId(textDataId);

	if (id == -1 || textId == -1 || !_widgets[id])
		return false;
	
	_widgets[id]->_textDataId = textId;
	_widgets[id]->_textAlignH = alignH;
	_widgets[id]->_textAlignV = alignV;	

	return true;
}

bool ThemeRenderer::addFont(const Common::String &fontId, int r, int g, int b) {
	TextData textId = getTextDataId(fontId);
	
	if (textId == -1)
		return false;
		
	if (_texts[textId] != 0)
		return false;
		
	_texts[textId] = new TextDrawData;
	
	// TODO: Allow the user to specify the font he wants, instead of choosing based on resolution
	_texts[textId]->_fontPtr = _font;
	
	_texts[textId]->_color.r = r;
	_texts[textId]->_color.g = g;
	_texts[textId]->_color.b = b;
	return true;
	
}

bool ThemeRenderer::addDrawData(const Common::String &data, bool cached) {
	DrawData data_id = getDrawDataId(data);

	if (data_id == -1 || _widgets[data_id] != 0)
		return false;

	_widgets[data_id] = new WidgetDrawData;
	_widgets[data_id]->_cached = cached;
	_widgets[data_id]->_buffer = kDrawDataDefaults[data_id].buffer;
	_widgets[data_id]->_surfaceCache = 0;
	_widgets[data_id]->_textDataId = -1;

	return true;
}

bool ThemeRenderer::loadTheme(Common::String themeName) {
	unloadTheme();

	if (themeName == "builtin" && !loadDefaultXML())
		error("Could not load default embeded theme.");

	if (!loadThemeXML(themeName)) {
		warning("Could not parse custom theme '%s'.\nFalling back to default theme", themeName.c_str());
		
		if (!loadDefaultXML()) // if we can't load the embeded theme, this is a complete failure
			error("Could not load default embeded theme");
	}

	for (int i = 0; i < kDrawDataMAX; ++i) {
		if (_widgets[i] == 0) {
			warning("Missing data asset: '%s'", kDrawDataDefaults[i].name);
		} else {
			calcBackgroundOffset((DrawData)i);

			// TODO: draw the cached widget to the cache surface
			if (_widgets[i]->_cached) {}
		}
	}
	
	// Debug print all the parsed variables. remove
	_themeEval->debugPrint();
	
	_themeOk = true;
	return true;
}

bool ThemeRenderer::loadDefaultXML() {
	
	// The default XML theme is included on runtime from a pregenerated
	// file inside the themes directory.
	// Use the Python script "makedeftheme.py" to convert a normal XML theme
	// into the "default.inc" file, which is ready to be included in the code.
	const char *defaultXML =
#include "themes/default.inc"
	;
	
	if (!parser()->loadBuffer((const byte*)defaultXML, strlen(defaultXML), false))
		return false;

	return parser()->parse();
}

bool ThemeRenderer::loadThemeXML(Common::String themeName) {
	assert(_parser);

	if (ConfMan.hasKey("themepath"))
		Common::File::addDefaultDirectory(ConfMan.get("themepath"));

#ifdef DATA_PATH
	Common::File::addDefaultDirectoryRecursive(DATA_PATH);
#endif

	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

	if (!parser()->loadFile(themeName + ".xml"))
		return false;
	
	return parser()->parse();
}

bool ThemeRenderer::isWidgetCached(DrawData type, const Common::Rect &r) {
	return _widgets[type] && _widgets[type]->_cached &&
		_widgets[type]->_surfaceCache->w == r.width() && 
		_widgets[type]->_surfaceCache->h == r.height();
}

void ThemeRenderer::drawCached(DrawData type, const Common::Rect &r) {
	assert(_widgets[type]->_surfaceCache->bytesPerPixel == _screen->bytesPerPixel);
	_vectorRenderer->blitSurface(_widgets[type]->_surfaceCache, r);
}

void ThemeRenderer::queueDD(DrawData type, const Common::Rect &r, uint32 dynamic) {
	if (_widgets[type] == 0)
		return;
		
	DrawQueue q;	
	q.type = type;
	q.area = r;
	q.area.clip(_screen->w, _screen->h);
	q.dynData = dynamic;
	
	if (_buffering) {
		if (_widgets[type]->_buffer) {
			_bufferQueue.push_back(q);	
		} else {
			if (kDrawDataDefaults[type].parent != kDDNone && kDrawDataDefaults[type].parent != type)
				queueDD(kDrawDataDefaults[type].parent, r);

			// HACK: text selection backgrounds must be drawn before other widgets, because
			// they are implemented poorly and they overlap.
			if (type == kDDTextSelectionBackground)
				_screenQueue.push_front(q);
			else
				_screenQueue.push_back(q);
		}
	} else {
		drawDD(q, !_widgets[type]->_buffer, _widgets[type]->_buffer);
	}
}

void ThemeRenderer::queueDDText(TextData type, const Common::Rect &r, const Common::String &text, bool restoreBg,
	bool elipsis, TextAlign alignH, TextAlignVertical alignV, int deltax) {
		
	if (_texts[type] == 0)
		return;
		
	DrawQueueText q;
	q.type = type;
	q.area = r;
	q.area.clip(_screen->w, _screen->h);
	q.text = text;
	q.alignH = alignH;
	q.alignV = alignV;
	q.restoreBg = restoreBg;
	q.deltax = deltax;
	
	if (_buffering) {		
		_textQueue.push_back(q);
	} else {
		drawDDText(q);
	}
}

void ThemeRenderer::drawDD(const DrawQueue &q, bool draw, bool restore) {
	Common::Rect extendedRect = q.area;
	extendedRect.grow(kDirtyRectangleThreshold);
	extendedRect.right += _widgets[q.type]->_backgroundOffset;
	extendedRect.bottom += _widgets[q.type]->_backgroundOffset;

	if (restore)
		restoreBackground(extendedRect);
		
	if (draw) {
		if (isWidgetCached(q.type, q.area)) {
			drawCached(q.type, q.area);
		} else {
			for (Common::List<Graphics::DrawStep>::const_iterator step = _widgets[q.type]->_steps.begin(); 
				 step != _widgets[q.type]->_steps.end(); ++step)
				_vectorRenderer->drawStep(q.area, *step, q.dynData);
		}
	}
	
	addDirtyRect(extendedRect);
}

void ThemeRenderer::drawDDText(const DrawQueueText &q) {	
	if (q.restoreBg)
		restoreBackground(q.area);
	
	_vectorRenderer->setFgColor(_texts[q.type]->_color.r, _texts[q.type]->_color.g, _texts[q.type]->_color.b);
	_vectorRenderer->drawString(_texts[q.type]->_fontPtr, q.text, q.area, q.alignH, q.alignV, q.deltax);
	addDirtyRect(q.area);
}

void ThemeRenderer::calcBackgroundOffset(DrawData type) {
	uint maxShadow = 0;
	for (Common::List<Graphics::DrawStep>::const_iterator step = _widgets[type]->_steps.begin(); 
		step != _widgets[type]->_steps.end(); ++step) {
		if (((*step).autoWidth || (*step).autoHeight) && (*step).shadow > maxShadow) 
			maxShadow = (*step).shadow;
	}

	_widgets[type]->_backgroundOffset = maxShadow;
}

void ThemeRenderer::restoreBackground(Common::Rect r, bool special) {
	r.clip(_screen->w, _screen->h); // AHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHA... Oh god. :(
	_vectorRenderer->blitSurface(_backBuffer, r);
}

void ThemeRenderer::drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, uint16 hints) {
	if (!ready())
		return;
		
	DrawData dd = kDDButtonIdle;

	if (state == kStateEnabled)
		dd = kDDButtonIdle;
	else if (state == kStateHighlight)
		dd = kDDButtonHover;
	else if (state == kStateDisabled)
		dd = kDDButtonDisabled;

	queueDD(dd, r);
	queueDDText(getTextData(dd), r, str, false, false, _widgets[dd]->_textAlignH, _widgets[dd]->_textAlignV);
}

void ThemeRenderer::drawLineSeparator(const Common::Rect &r, WidgetStateInfo state) {
	if (!ready())
		return;

	queueDD(kDDSeparator, r);
}

void ThemeRenderer::drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state) {
	if (!ready())
		return;

	Common::Rect r2 = r;
	DrawData dd = kDDCheckboxDefault;
	
	if (checked)
		dd = kDDCheckboxSelected;
		
	if (state == kStateDisabled)
		dd = kDDCheckboxDisabled;
	
	TextData td = (state == kStateHighlight) ? kTextDataHover : getTextData(dd);
	const int checkBoxSize = MIN((int)r.height(), getFontHeight());

	r2.bottom = r2.top + checkBoxSize;
	r2.right = r2.left + checkBoxSize;

	queueDD(dd, r2);
	
	r2.left = r2.right + checkBoxSize;
	r2.right = r.right;
	
	queueDDText(td, r2, str, false, false, _widgets[kDDCheckboxDefault]->_textAlignH, _widgets[dd]->_textAlignV);
}

void ThemeRenderer::drawSlider(const Common::Rect &r, int width, WidgetStateInfo state) {
	if (!ready())
		return;
		
	DrawData dd = kDDSliderFull;
	
	if (state == kStateHighlight)
		dd = kDDSliderHover;
	else if (state == kStateDisabled)
		dd = kDDSliderDisabled;

	Common::Rect r2 = r;
	r2.setWidth(MIN((int16)width, r.width()));
	r2.top++; r2.bottom--; r2.left++; r2.right--;

	drawWidgetBackground(r, 0, kWidgetBackgroundSlider, kStateEnabled);
	
	if (width > r.width() * 5 / 100)
		queueDD(dd, r2);
}

void ThemeRenderer::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState scrollState, WidgetStateInfo state) {
	if (!ready())
		return;
		
	queueDD(kDDScrollbarBase, r);
	
	Common::Rect r2 = r;
	const int buttonExtra = (r.width() * 120) / 100;
	
	r2.bottom = r2.top + buttonExtra;
	queueDD(scrollState == kScrollbarStateUp ? kDDScrollbarButtonHover : kDDScrollbarButtonIdle, r2, Graphics::VectorRenderer::kTriangleUp);
	
	r2.translate(0, r.height() - r2.height());
	queueDD(scrollState == kScrollbarStateDown ? kDDScrollbarButtonHover : kDDScrollbarButtonIdle, r2, Graphics::VectorRenderer::kTriangleDown);
	
	r2 = r;
	r2.left += 1;
	r2.right -= 1;
	r2.top += sliderY;
	r2.bottom = r2.top + sliderHeight - 1;
	queueDD(scrollState == kScrollbarStateSlider ? kDDScrollbarHandleHover : kDDScrollbarHandleIdle, r2);
}

void ThemeRenderer::drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state) {
	if (!ready())
		return;
		
	if (hints & THEME_HINT_MAIN_DIALOG) {
		queueDD(kDDMainDialogBackground, r);
	} else if (hints & THEME_HINT_SPECIAL_COLOR) { 
		queueDD(kDDSpecialColorBackground, r);
	} else if (hints & THEME_HINT_PLAIN_COLOR) {
		queueDD(kDDPlainColorBackground, r);	
	} else {
		queueDD(kDDDefaultBackground, r);
	}
}

void ThemeRenderer::drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state) {
	if (!ready())
		return;

	debugWidgetPosition("Caret", r);
}

void ThemeRenderer::drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state, TextAlign align) {
	if (!ready())
		return;
		
	DrawData dd = (state == kStateHighlight) ? kDDPopUpHover : kDDPopUpIdle;
	
	queueDD(dd, r);
	
	if (!sel.empty()) {
		Common::Rect text(r.left, r.top, r.right - 16, r.bottom);
		queueDDText(getTextData(dd), text, sel, false, false, _widgets[dd]->_textAlignH, _widgets[dd]->_textAlignV);
	}
}

void ThemeRenderer::drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans) {
	if (!ready())
		return;

	debugWidgetPosition("Surface", r);
}

void ThemeRenderer::drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, WidgetStateInfo state) {
	if (!ready())
		return;
		
	switch (background) {
	case kWidgetBackgroundBorderSmall:
		queueDD(kDDWidgetBackgroundSmall, r);
		break;
		
	case kWidgetBackgroundEditText:
		queueDD(kDDWidgetBackgroundEditText, r);
		break;
		
	case kWidgetBackgroundSlider:
		queueDD(kDDWidgetBackgroundSlider, r);
		break;
		
	default:
		queueDD(kDDWidgetBackgroundDefault, r);
		break;
	}
}

void ThemeRenderer::drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state) {
	if (!ready())
		return;
		
	const int tabOffset = 2;
	tabWidth -= tabOffset;
	
	queueDD(kDDTabBackground, Common::Rect(r.left, r.top, r.right, r.top + tabHeight));
	
	for (int i = 0; i < (int)tabs.size(); ++i) {
		if (i == active)
			continue;
	
		Common::Rect tabRect(r.left + i * (tabWidth + tabOffset), r.top, r.left + i * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight);
		queueDD(kDDTabInactive, tabRect);
		queueDDText(getTextData(kDDTabInactive), tabRect, tabs[i], false, false, _widgets[kDDTabInactive]->_textAlignH, _widgets[kDDTabInactive]->_textAlignV);
	}
	
	if (active >= 0) {
		Common::Rect tabRect(r.left + active * (tabWidth + tabOffset), r.top, r.left + active * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight);
		const uint16 tabLeft = active * (tabWidth + tabOffset);
		const uint16 tabRight =  MAX(r.right - tabRect.right, 0);
		queueDD(kDDTabActive, tabRect, (tabLeft << 16) | (tabRight & 0xFFFF));
		queueDDText(getTextData(kDDTabActive), tabRect, tabs[active], false, false, _widgets[kDDTabActive]->_textAlignH, _widgets[kDDTabActive]->_textAlignV);
	}
}

void ThemeRenderer::drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font) {
	if (!ready())
		return;
		
	if (inverted) {
		queueDD(kDDTextSelectionBackground, r);
		queueDDText(kTextDataInverted, r, str, false, useEllipsis, align);
		return;
	}

	switch (state) {
		case kStateDisabled:
			queueDDText(kTextDataDisabled, r, str, true, useEllipsis, align);
			break;
			
		case kStateHighlight:
			queueDDText(kTextDataHover, r, str, true, useEllipsis, align);
			break;
		
		case kStateEnabled:
			queueDDText(kTextDataDefault, r, str, true, useEllipsis, align);
			break;
	}
}

void ThemeRenderer::debugWidgetPosition(const char *name, const Common::Rect &r) {
	_font->drawString(_screen, name, r.left, r.top, r.width(), 0xFFFF, Graphics::kTextAlignRight, 0, true);
	_screen->hLine(r.left, r.top, r.right, 0xFFFF);
	_screen->hLine(r.left, r.bottom, r.right, 0xFFFF);
	_screen->vLine(r.left, r.top, r.bottom, 0xFFFF);
	_screen->vLine(r.right, r.top, r.bottom, 0xFFFF);
}

void ThemeRenderer::updateScreen() {
	if (!_bufferQueue.empty()) {
		_vectorRenderer->setSurface(_backBuffer);
		
		for (Common::List<DrawQueue>::const_iterator q = _bufferQueue.begin(); q != _bufferQueue.end(); ++q)
			drawDD(*q, true, false);
			
		_vectorRenderer->setSurface(_screen);
		_vectorRenderer->blitSurface(_backBuffer, Common::Rect(0, 0, _screen->w, _screen->h));
		_bufferQueue.clear();
	}
	
	if (!_screenQueue.empty()) {
		_vectorRenderer->disableShadows();
		for (Common::List<DrawQueue>::const_iterator q = _screenQueue.begin(); q != _screenQueue.end(); ++q)
			drawDD(*q, true, false);
			
		_vectorRenderer->enableShadows();
		_screenQueue.clear();
	}
	
	if (!_textQueue.empty()) {
		for (Common::List<DrawQueueText>::const_iterator q = _textQueue.begin(); q != _textQueue.end(); ++q)
			drawDDText(*q);
			
		_textQueue.clear();
	}
		
	renderDirtyScreen();
//	_vectorRenderer->copyWholeFrame(_system);
}

void ThemeRenderer::renderDirtyScreen() {
	if (_dirtyScreen.empty())
		return;

	Common::List<Common::Rect>::const_iterator i, j;
	for (i = _dirtyScreen.begin(); i != _dirtyScreen.end(); ++i) {
		for (j = i; j != _dirtyScreen.end(); ++j)
			if (j != i && i->contains(*j))
				j = _dirtyScreen.reverse_erase(j);

		_vectorRenderer->copyFrame(_system, *i);
	}
		
	_dirtyScreen.clear();
}

void ThemeRenderer::openDialog(bool doBuffer, ShadingStyle style) {
	if (doBuffer)
		_buffering = true;
		
	if (style != kShadingNone) {
		_vectorRenderer->applyScreenShading(style);
		addDirtyRect(Common::Rect(0, 0, _screen->w, _screen->h));
	}

	_vectorRenderer->setSurface(_backBuffer);
	_vectorRenderer->blitSurface(_screen, Common::Rect(0, 0, _screen->w, _screen->h));
	_vectorRenderer->setSurface(_screen);
}

} // end of namespace GUI.
