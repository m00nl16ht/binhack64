/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	bincon patch engine, from dopefish's 0WINCEOS.BIN -> 1ST_READ.BIN
	Converter v2.
*/

#ifndef __BINCON__HPP__
#define __BINCON__HPP__

#include "binhack.hpp"

#define BINCON_CHUNK_SIZE 2048

// bincon by dopefish: voodoo way to make WinCE games boot. Rewrites boot
// in place as original[chunk:N] followed by original[N-chunk:N] - the
// first chunk is dropped and the last chunk ends up duplicated at the end
// (not a simple "rotate first chunk to the end").
//
// Unlike the original tool, this only ever touches the one boot file
// passed to it - copy it first to keep the pre-mastered original, and run
// binhack-ip afterward for the IP.BIN side.
//
// Returns false, leaving boot untouched, if the file is already Katana,
// already bincon'd, bincon'd-but-not-WinCE, or too small to transform.
bool applyBincon(std::fstream& boot, unsigned int bootsize);

#endif // __BINCON__HPP__
