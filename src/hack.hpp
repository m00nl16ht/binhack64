/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	HACK0 / HACK / HACK2 / HACK3 / DAHACK patch engine: alternate
	boot-binary-only LBA patches, distinct from the classic BINHACK patch
	in binhack.hpp. None of these touch IP.BIN. hack/hack2/hack3/dahack are
	ported from binhacks.py; hack0 from kikuchan's hack4 v1.5 (2001), which
	also bundles hack1/hack2/hack3 under those same names.
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

// Scans boot for every non-overlapping little-endian uint32 occurrence of
// oldValue and overwrites it with newValue. Returns the number of
// replacements made (0 is not an error - the pattern just wasn't found).
unsigned int searchReplaceUint32LE(fstream& boot, unsigned int bootsize,
                                    unsigned int oldValue, unsigned int newValue);

// HACK0, from kikuchan's hack4: replaces every raw oldLba reference with
// lba directly (no +166/+150 offset).
unsigned int applyHack0(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// HACK by Bero: replaces every (oldLba + 166) reference with (lba + 166).
unsigned int applyHack(fstream& boot, unsigned int bootsize,
                       unsigned int lba, unsigned int oldLba);

// HACK2 by Unknown: replaces every (oldLba + 150) reference with (lba + 150).
unsigned int applyHack2(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// HACK3, original exe by Pekearai: HACK(lba) + HACK2(lba).
unsigned int applyHack3(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba);

// DAHACK by Mr. KiMWU: HACK(lba) + HACK2(0).
unsigned int applyDahack(fstream& boot, unsigned int bootsize,
                         unsigned int lba, unsigned int oldLba);

#endif // __HACK__HPP__
