/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	unprotect patch engine: 7 non-LBA copy-protection cracks, ported from
	binhacks.py. Also backs the read-only check-protection command via
	checkProtection (binhacks.py's isProtected/isUnProtected).
*/

#ifndef __UNPROTECT__HPP__
#define __UNPROTECT__HPP__

#include "hack.hpp"
#include <string>

using namespace std;

#define UNPROTECT_VARIANT_COUNT 7

// Parses a CLI id argument ("0".."6" or "jsr", an alias for 6) into a
// variant index, or -1 if invalid.
int unprotectVariantFromString(const string& arg);

// Applies unprotect variant (0-6) to boot, reporting the replacement
// count via count. Returns false (count untouched) for an out-of-range
// variant.
bool applyUnprotect(fstream& boot, unsigned int bootsize, int variant, unsigned int& count);

// Reports whether variant's original (isProtected) and/or cracked
// (isUnprotected) pattern is present in boot. Returns false (outputs
// untouched) for an out-of-range variant.
bool checkProtection(istream& boot, unsigned int bootsize, int variant,
                     bool& isProtected, bool& isUnprotected);

// Short credit string for variant (e.g. "Mr.talon"), or "" if out of range.
string unprotectVariantCredit(int variant);

#endif // __UNPROTECT__HPP__
