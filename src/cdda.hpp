/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	CDDA fix patch engine.
*/

#ifndef __CDDA__HPP__
#define __CDDA__HPP__

#include "binhack.hpp"

// Magic constant subtracted from the marker value read near CD001 to get
// the write offset. Deliberately not 0x18C - it's a byte-order artifact
// of the original tool, kept as-is to match its behavior.
#define CDDA_OFFSET_MAGIC 0x8C010000u

// CDDA fix, method posted by Mr.KiMWU: fixes multi-track CDDA bootbins
// where the first audio track reads as track04 instead of track01, by
// writing a small routine plus two numeric patches near the disc's first
// CD001 signature.
//
// Unlike the original, returns false and leaves boot untouched if the
// computed offset doesn't look sane for this file.
bool applyCdda(std::fstream& boot, unsigned int bootsize);

#endif // __CDDA__HPP__
