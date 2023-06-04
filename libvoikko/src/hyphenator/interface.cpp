/* The contents of this file are subject to the Mozilla Public License Version 
 * 1.1 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at 
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * 
 * The Original Code is Libvoikko: Library of natural language processing tools.
 * The Initial Developer of the Original Code is Harri Pitkänen <hatapitk@iki.fi>.
 * Portions created by the Initial Developer are Copyright (C) 2006 - 2010
 * the Initial Developer. All Rights Reserved.
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *********************************************************************************/

#include "setup/setup.hpp"
#include "utils/StringUtils.hpp"
#include "hyphenator/Hyphenator.hpp"
#include "porting.h"
#include <cstdlib>
#include <sstream>

namespace libvoikko { namespace hyphenator {

VOIKKOEXPORT char * voikkoHyphenateUcs4(voikko_options_t * options, const wchar_t * word) {
	if (word == 0) {
		return 0;
	}
	size_t wlen = wcslen(word);
	
	char * hyphenation = options->hyphenator->hyphenate(word, wlen);
	if (hyphenation == 0) {
		return 0;
	}
	return hyphenation;
}

VOIKKOEXPORT char * voikkoHyphenateCstr(voikko_options_t * options, const char * word) {
	if (word == 0) {
		return 0;
	}
	size_t len = strlen(word);
	if (len > LIBVOIKKO_MAX_WORD_CHARS) {
		return 0;
	}
	wchar_t * word_ucs4 = utils::StringUtils::ucs4FromUtf8(word, len);
	if (word_ucs4 == 0) {
		return 0;
	}
	char * result = voikkoHyphenateUcs4(options, word_ucs4);
	delete[] word_ucs4;
	return result;
}

VOIKKOEXPORT char * voikkoInsertHyphensCstr(voikko_options_t * options, const char * word,
                                            const char * hyphen, int allowContextChanges) {
	if (!word) {
		return nullptr;
	}

	// For now we are intentionally omitting UCS4 variant of this function and thus
	// we perform the necessary initial conversions here.
	size_t len = strlen(word);
	if (len > LIBVOIKKO_MAX_WORD_CHARS) {
		return nullptr;
	}
	wchar_t * word_ucs4 = utils::StringUtils::ucs4FromUtf8(word, len);
	if (!word_ucs4) {
		return nullptr;
	}
	wchar_t * hyphen_ucs4 = utils::StringUtils::ucs4FromUtf8(hyphen, strlen(hyphen));
	if (!hyphen_ucs4) {
		delete[] word_ucs4;
		return nullptr;
	}

	// Create the hyphenated form
	char * hyphenationPattern = voikkoHyphenateUcs4(options, word_ucs4);
	if (!hyphenationPattern) {
		return nullptr;
	}
	std::wstringstream hyphenated;
	size_t patternLen = strlen(hyphenationPattern);
	for (size_t i = 0; i < patternLen; i++) {
		char patternC = hyphenationPattern[i];
		if (patternC == '-') {
			hyphenated << hyphen_ucs4 << word_ucs4[i];
		}
		else if (patternC == ' ' || !allowContextChanges) {
			hyphenated << word_ucs4[i];
		}
		else if (patternC == '=') {
			if (word_ucs4[i] == L'-') {
				hyphenated << L'-';
			}
			else {
				hyphenated << hyphen_ucs4;
			}
		}
	}

	// Clean up and return the result
	delete[] hyphen_ucs4;
	delete[] word_ucs4;
	delete[] hyphenationPattern;
	return utils::StringUtils::utf8FromUcs4(hyphenated.str().c_str());
}

VOIKKOEXPORT void voikkoFreeCstr(char * cstr) {
	delete[] cstr;
}

} }
