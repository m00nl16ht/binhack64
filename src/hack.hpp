/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	HACK0 / HACK / HACK2 / HACK3 / DAHACK patch engine: alternate
	boot-binary-only LBA patches, distinct from the classic BINHACK patch
	in binhack.hpp. None of these touch IP.BIN.

	Also hosts searchReplaceBytes/containsBytes, generic byte-pattern
	primitives reused by unprotect.cpp.

	See CREDITS.md for authorship and dates.
*/

#ifndef __HACK__HPP__
#define __HACK__HPP__

#include <fstream>

// Default "old LBA" the numeric patterns below are computed against:
// how most original discs were mastered before being re-burned.
#define HACK_DEFAULT_OLD_LBA 45000

#define HACK_LBA_OFFSET 166   // HACK targets (oldLba + 166)
#define HACK2_LBA_OFFSET 150  // HACK2 targets (oldLba + 150)

// Scans boot for every non-overlapping occurrence of oldPattern (patternLen
// bytes) and overwrites it with newPattern. Returns the number of
// replacements made (0 is not an error - the pattern just wasn't found).
unsigned int searchReplaceBytes(std::fstream& boot, unsigned int bootsize,
                                const unsigned char* oldPattern, const unsigned char* newPattern,
                                unsigned int patternLen);

// True if pattern (patternLen bytes) occurs anywhere in boot.
bool containsBytes(std::istream& boot, unsigned int bootsize,
                   const unsigned char* pattern, unsigned int patternLen);

// 4-byte little-endian uint32 convenience wrapper over searchReplaceBytes.
unsigned int searchReplaceUint32LE(std::fstream& boot, unsigned int bootsize,
                                    unsigned int oldValue, unsigned int newValue);

// HACK0, from kikuchan's hack4: replaces every raw oldLba reference with
// lba directly (no +166/+150 offset).
unsigned int applyHack0(std::fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// HACK1 by Bero: replaces every (oldLba + 166) reference with (lba + 166).
unsigned int applyHack(std::fstream& boot, unsigned int bootsize,
                       unsigned int lba, unsigned int oldLba);

// HACK2 by Unknown: replaces every (oldLba + 150) reference with (lba + 150).
unsigned int applyHack2(std::fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// HACK3 by Pekearai / Mr. KiMWU: HACK1(lba) + HACK2(lba).
unsigned int applyHack3(std::fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// DAHACK, method posted by Mr.KiMWU: HACK1(lba) + HACK2(0).
unsigned int applyDahack(std::fstream& boot, unsigned int bootsize,
                         unsigned int lba, unsigned int oldLba);

#endif // __HACK__HPP__
