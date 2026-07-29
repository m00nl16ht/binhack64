/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	BINHACK patch commands: mid-level building blocks and high-level
	command entry points for the classic BINHACK patch (LBA hack + IP.BIN
	bootstrap rewrite). The mid-level functions are exposed here, not
	hidden as static in commands.cpp, so the interactive CLI (cli.cpp) can
	interleave its prompts between them the same way the legacy tool did.
*/

#ifndef __COMMANDS__HPP__
#define __COMMANDS__HPP__

#include "binhack.hpp"
#include "hack.hpp"
#include "cdda.hpp"
#include "bincon.hpp"
#include "unprotect.hpp"
#include <string>

using namespace std;

// -----------------------------------------------------------------------------
// EXIT CODES
// -----------------------------------------------------------------------------

namespace ExitCode {
    const int Ok = 0;
    const int UsageError = 1;
    const int BootOpenError = 2;
    const int BootHackError = 3;
    const int IpOpenError = 4;
    const int IpWriteError = 5;
}

// -----------------------------------------------------------------------------
// MID-LEVEL BUILDING BLOCKS
// -----------------------------------------------------------------------------

// Opens bootname read/write and locates all CD001 offsets in it.
bool processBootBin(const string& bootname, unsigned int& bootsize,
                    vector<unsigned int>& hackoffsets, fstream& boot);

// Applies the LBA hack to boot at every offset found by processBootBin
// (no-op for Windows CE binaries, which don't need it).
bool hackBootBin(fstream& boot, const vector<unsigned int>& hackoffsets,
                 unsigned int lba, bool& isWinCEBinary);

// Loads the unpatched IP.BIN template (always BOOTSECTOR_NAME in the
// current directory) into buffer.
bool loadIpBin(char* buffer);

// Writes ipname from iphackbuf, patched with region/VGA/bincon flags and
// the BOOT.BIN size. boot is only read (for the bincon check), never written.
bool createHackedIpBin(const string& ipname, char* iphackbuf,
                       unsigned int bootsize, fstream& boot);

// -----------------------------------------------------------------------------
// HIGH-LEVEL COMMANDS
// -----------------------------------------------------------------------------

// Patch BOOT.BIN (1ST_READ.BIN) only: writes the LBA hack. IP.BIN is
// neither read nor written.
int runBinhackBoot(const string& bootname, unsigned int lba);

// Patch IP.BIN only: writes region/VGA/bincon flags and the BOOT.BIN size.
// bootname is opened read-only and never modified.
int runBinhackIp(const string& bootname, const string& ipname);

// Patch both BOOT.BIN and IP.BIN (classic BINHACK behavior).
int runBinhack(const string& bootname, const string& ipname, unsigned int lba);

// HACK0 (kikuchan), HACK (Bero), HACK2 (Unknown), HACK3 (Pekearai) and
// DAHACK (Mr. KiMWU): alternate boot-binary-only LBA patches. None of
// these touch IP.BIN.
int runHack0(const string& bootname, unsigned int lba, unsigned int oldLba);
int runHack(const string& bootname, unsigned int lba, unsigned int oldLba);
int runHack2(const string& bootname, unsigned int lba, unsigned int oldLba);
int runHack3(const string& bootname, unsigned int lba, unsigned int oldLba);
int runDahack(const string& bootname, unsigned int lba, unsigned int oldLba);

// CDDA fix (Mr. KiMWU): fixes multi-track CDDA bootbins. Doesn't need an
// LBA - it locates itself from the boot binary's own CD001 signature.
int runCdda(const string& bootname);

// bincon (dopefish/echelon): voodoo to make WinCE games boot.
int runBincon(const string& bootname);

// unprotect: removes one of 7 non-LBA copy protections (variant 0-6).
int runUnprotect(const string& bootname, int variant);

// check-protection: read-only report of whether variant's original
// and/or cracked pattern is present. variant -1 checks all 7.
int runCheckProtection(const string& bootname, int variant);

#endif // __COMMANDS__HPP__
