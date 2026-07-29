/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	HACK0 / HACK / HACK2 / HACK3 / DAHACK patch engine: alternate
	boot-binary-only LBA patches, distinct from the classic BINHACK patch
	in binhack.hpp. None of these touch IP.BIN. hack/hack2/hack3/dahack are
	ported from binhacks.py; hack0 from kikuchan's hack4, which also
	bundles hack1/hack2/hack3 under those same names. hack4.exe's own
	binary embeds the CVS tag "hack4.c,v 1.5 2001/05/04 19:40:44 kikuchan
	Exp $", confirming both the version and date. hack3.exe/dahack.exe are
	both dated 2001-02-23; the original toolkit's readme documents DAHACK
	as using "the method which post by Mr.KiMWU," confirming that
	attribution independently of binhacks.py's docstring.

	Also hosts searchReplaceBytes/containsBytes, generic byte-pattern
	primitives reused by unprotect.cpp (searchReplaceUint32LE below is
	just a 4-byte convenience wrapper over the same primitive).
*/

#ifndef __HACK__HPP__
#define __HACK__HPP__

#include <fstream>

using namespace std;

// Default "old LBA" the numeric patterns below are computed against,
// matching binhacks.py's oldLBA=45000 default (i.e. how most original
// discs were mastered before being re-burned at a new LBA).
#define HACK_DEFAULT_OLD_LBA 45000

#define HACK_LBA_OFFSET 166   // HACK targets (oldLba + 166)
#define HACK2_LBA_OFFSET 150  // HACK2 targets (oldLba + 150)

// Scans boot for every non-overlapping occurrence of oldPattern (patternLen
// bytes) and overwrites it with newPattern. Returns the number of
// replacements made (0 is not an error - the pattern just wasn't found).
unsigned int searchReplaceBytes(fstream& boot, unsigned int bootsize,
                                const unsigned char* oldPattern, const unsigned char* newPattern,
                                unsigned int patternLen);

// True if pattern (patternLen bytes) occurs anywhere in boot.
bool containsBytes(istream& boot, unsigned int bootsize,
                   const unsigned char* pattern, unsigned int patternLen);

// 4-byte little-endian uint32 convenience wrapper over searchReplaceBytes.
unsigned int searchReplaceUint32LE(fstream& boot, unsigned int bootsize,
                                    unsigned int oldValue, unsigned int newValue);

// HACK0, from kikuchan's hack4 v1.5 (2001/05/04): replaces every raw
// oldLba reference with lba directly (no +166/+150 offset).
unsigned int applyHack0(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// HACK by Bero: replaces every (oldLba + 166) reference with (lba + 166).
unsigned int applyHack(fstream& boot, unsigned int bootsize,
                       unsigned int lba, unsigned int oldLba);

// HACK2 by Unknown: replaces every (oldLba + 150) reference with (lba + 150).
unsigned int applyHack2(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// HACK3, original exe by Pekearai (2001-02-23): HACK(lba) + HACK2(lba).
unsigned int applyHack3(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// DAHACK, method posted by Mr.KiMWU (exe dated 2001-02-23): HACK(lba) + HACK2(0).
unsigned int applyDahack(fstream& boot, unsigned int bootsize,
                         unsigned int lba, unsigned int oldLba);

#endif // __HACK__HPP__
