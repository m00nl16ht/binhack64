/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	CDDA fix patch engine, ported from binhacks.py.
*/

#ifndef __CDDA__HPP__
#define __CDDA__HPP__

#include "binhack.hpp"

// Magic constant subtracted from the marker value read near CD001 to get
// the write offset. Not simply 0x18C - it's a byte-order artifact of the
// original Python (str2num('0000018c') round-trips a big-endian pack
// through a little-endian unpack); verified empirically against a real
// CDDA bootbin fixture rather than re-derived from the Python source.
#define CDDA_OFFSET_MAGIC 0x8C010000u

// CDDA fix, method posted by Mr.KiMWU (cdda.exe dated 2001-02-23, per the
// original toolkit's readme): fixes multi-track CDDA bootbins where the
// first audio track reads as track04 instead of track01, by writing a
// small routine plus two numeric patches near the disc's first CD001
// signature.
// Unlike the original (which had no such check, by its own admission),
// this returns false and leaves boot untouched if the computed offset
// doesn't look sane for this file.
bool applyCdda(fstream& boot, unsigned int bootsize);

#endif // __CDDA__HPP__
