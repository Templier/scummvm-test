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

#include "translation.h"

DECLARE_SINGLETON(Common::TranslationManager)

#ifdef USE_DETECTLANG
#include <locale.h>
#endif

#ifdef USE_TRANSLATION
#include "messages.cpp"
#endif

namespace Common {


#ifdef USE_TRANSLATION

// Translation enabled

TranslationManager::TranslationManager() {
#ifdef USE_DETECTLANG
	// Activating current locale settings
	const char *locale = setlocale(LC_ALL, "");

	// Detect the language from the locale
	if (!locale) {
		_syslang = "C";
	} else {
		int length = 0;

		// Strip out additional information, like
		// ".UTF-8" or the like. We do this, since
		// our translation languages are usually
		// specified without any charset information.
		for (int i = 0; locale[i]; ++i) {
			// TODO: Check whether "@" should really be checked
			// here.
			if (locale[i] == '.' || locale[i] == ' ' || locale[i] == '@') {
				length = i;
				break;
			}

			length = i;
		}

		_syslang = String(locale, length);
	}
#else // USE_DETECTLANG
	_syslang = "C";
#endif // USE_DETECTLANG

	// Set the default language
	setLanguage("");
}

TranslationManager::~TranslationManager() {
}

void TranslationManager::setLanguage(const char *lang) {
	if (*lang == '\0')
		po2c_setlang(_syslang.c_str());
	else
		po2c_setlang(lang);
}

const char *TranslationManager::getTranslation(const char *message) {
	return po2c_gettext(message);
}

String TranslationManager::getTranslation(const String &message) {
	return po2c_gettext(message.c_str());
}

const TLangArray TranslationManager::getSupportedLanguages() const {
	TLangArray languages;

	int total = po2c_getnumlangs();
	for (int i = 0; i < total; i++) {
		TLanguage lng(po2c_getlang(i), i + 1);
		languages.push_back(lng);
	}

	//sort(languages.begin(), languages.end());

	return languages;
}

int TranslationManager::parseLanguage(const String lang) {
	int total = po2c_getnumlangs();

	for (int i = 0; i < total; i++) {
		if (lang == po2c_getlang(i))
			return i + 1;
	}

	return kTranslationBuiltinId;
}


const char *TranslationManager::getLangById(int id) {
	switch (id) {
	case kTranslationAutodetectId:
		return "";
	case kTranslationBuiltinId:
		return "C";
	default:
		if (id >= 0 && id - 1 < po2c_getnumlangs())
			return po2c_getlang(id - 1);
	}

	// In case an invalid ID was specified, we will output a warning
	// and return the same value as the auto detection id.
	warning("Invalid language id %d passed to TranslationManager::getLangById", id);
	return "";
}

#else // USE_TRANSLATION

// Translation disabled


TranslationManager::TranslationManager() {}

TranslationManager::~TranslationManager() {}

void TranslationManager::setLanguage(const char *lang) {}

const char *TranslationManager::getLangById(int id) {
	return "";
}

int TranslationManager::parseLanguage(const String lang) {
	return kTranslationBuiltinId;
}

const char *TranslationManager::getTranslation(const char *message) {
	return message;
}

String TranslationManager::getTranslation(const String &message) {
	return message;
}

const TLangArray TranslationManager::getSupportedLanguages() const {
	return TLangArray();
}

#endif // USE_TRANSLATION

}	// End of namespace Common