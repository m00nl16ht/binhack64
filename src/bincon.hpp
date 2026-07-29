/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	bincon patch engine. binhacks.py wraps this as bootbin.bincon(); the
	transform itself is ported from and verified against dopefish's
	original 0WINCEOS.BIN -> 1ST_READ.BIN Converter v2 (bincon.c, 7/28/00)
	- its fread/fwrite calls confirm the same drop-first-duplicate-last
	transform binhacks.py's write() calls resolve to, independent of that
	Python source. bincon.c was later given a basic already-converted
	check by Shoometsu (1/26/08); that specific check isn't ported here
	(see the "Preconditions" note below) - only dopefish's transform is.
*/

#ifndef __BINCON__HPP__
#define __BINCON__HPP__

#include "binhack.hpp"

#define BINCON_CHUNK_SIZE 2048

// bincon by dopefish, originally from echelon: voodoo way to make WinCE
// games boot. Rewrites boot in place as original[chunk:N] followed by
// original[N-chunk:N] - the first chunk is dropped and the last chunk
// ends up duplicated at the end (not a simple "rotate first chunk to the
// end"). The original tool reads a separate 0WINCEOS.BIN and writes a
// separate 1ST_READ.BIN plus patches IP.BIN's OS flag in the same run;
// here bincon only ever touches the one boot file passed to it (rewrites
// it in place) - copy it to a new name first if you want to keep the
// pre-mastered original, and run binhack-ip afterward for the IP.BIN side
// (it detects bincon'd files automatically, see isBincon in binhack.hpp).
//
// Preconditions use isWinCE/isBincon (this codebase's existing detection,
// also used by binhack-ip) rather than Shoometsu's 2008 already-converted
// check (comparing the input's last two 2KB chunks) added to bincon.c -
// that specific check was not ported.
//
// Returns false (leaving boot untouched) and prints the matching error
// if the precondition isn't met (already Katana, already bincon'd, or
// bincon'd-but-not-WinCE) or the file is too small for the transform to
// be meaningful (bootsize <= BINCON_CHUNK_SIZE) - the original tools have
// no such size check.
bool applyBincon(fstream& boot, unsigned int bootsize);

#endif // __BINCON__HPP__
