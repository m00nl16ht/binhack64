/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	WinCE+CDDA IP.BIN fix implementation.
*/

#include "wince_cdda_fix.hpp"
#include <cstring>

using namespace std;

namespace {
    const unsigned char oldBytes[2] = { 0x25, 0xC7 };
    const unsigned char newBytes[2] = { 0x09, 0x00 };
}

WinceCddaFixResult applyWinceCddaFix(fstream& ip, unsigned int ipsize) {
    if (ipsize < WINCE_CDDA_FIX_OFFSET + 2) {
        return WinceCddaFixResult::Unexpected;
    }

    unsigned char current[2];
    ip.seekg(WINCE_CDDA_FIX_OFFSET, ios::beg);
    ip.read(reinterpret_cast<char*>(current), 2);

    if (memcmp(current, oldBytes, 2) == 0) {
        ip.seekp(WINCE_CDDA_FIX_OFFSET, ios::beg);
        ip.write(reinterpret_cast<const char*>(newBytes), 2);
        return WinceCddaFixResult::Applied;
    }

    if (memcmp(current, newBytes, 2) == 0) {
        return WinceCddaFixResult::AlreadyApplied;
    }

    return WinceCddaFixResult::Unexpected;
}
