/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	WinCE+CDDA IP.BIN fix by pitito. Fixes WinCE games whose CDDA audio
	breaks when converted from GDI to CDI (typically always replaying
	track 1).

	Unlike every other patch in this codebase, this one targets IP.BIN
	itself, not the boot binary - the affected byte sits inside the
	bootstrap payload the classic BINHACK patch writes (see
	BOOTSECTOR_HACK_OFFSET in binhack.hpp), not in a vanilla IP.BIN, so it
	only makes sense to run after binhack-ip/binhack.
*/

#ifndef __WINCE_CDDA_FIX__HPP__
#define __WINCE_CDDA_FIX__HPP__

#include <fstream>

#define WINCE_CDDA_FIX_OFFSET 0x61C0

enum class WinceCddaFixResult {
    Applied,         // was 25 C7, now written as 09 00
    AlreadyApplied,  // was already 09 00 - no-op, not an error
    Unexpected       // neither pattern found - refused, ip left untouched
};

// Reads the 2 bytes at WINCE_CDDA_FIX_OFFSET in ip and classifies/applies
// per the three cases above. Also returns Unexpected (without reading) if
// ipsize is too small for the offset to exist.
WinceCddaFixResult applyWinceCddaFix(std::fstream& ip, unsigned int ipsize);

#endif // __WINCE_CDDA_FIX__HPP__
