/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Low-level BINHACK patch engine: data definitions and prototypes.
	binhack64 is a modernized, 64-bit-native continuation of binhack32
	(FamilyGuy and contributors, since 2010), itself a clone of the
	original 16-bit BINHACK.EXE. This file only covers the classic BINHACK patch
	(LBA hack + IP.BIN bootstrap rewrite); other patches get their own
	engine files.
*/

#ifndef __BINHACK__HPP__
#define __BINHACK__HPP__

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Prototypes
int filesize( istream& );
bool isWinCE( istream&, unsigned int );
vector<unsigned int> searchHackOffsets( istream& boot, unsigned int bootsize );
void hackBootStrap( ofstream&, unsigned int, fstream& );
bool hackKatanaBootBinary( fstream&, unsigned int, unsigned int );

// -----------------------------------------------------------------------------
// DATA DEFINITIONS
// -----------------------------------------------------------------------------

// Boot binary
#define BOOT_HACK_SIGNATURE_SIZE 5
const unsigned char bootsign_ref[ BOOT_HACK_SIGNATURE_SIZE ] = { 'C', 'D', '0', '0', '1' };

#define BOOT_HACK_WINCE_CHECK_SIZE 4
const unsigned char wincecheck_ref[ BOOT_HACK_WINCE_CHECK_SIZE ] = { 0x0D, 0x00, 0x0A, 0x00 };

#define BOOT_HACK_BINCON_CHECK_SIZE 2
const unsigned char binconcheck_ref[ BOOT_HACK_BINCON_CHECK_SIZE ] = { 0x09, 0x00 };

// Bootstrap
#define BOOTSECTOR_NAME "IP.BIN"
#define BOOTSECTOR_SIZE 0x8000

#define BOOTSECTOR_HACK_REGION_FLAGS_OFFSET 0x30
#define BOOTSECTOR_REGION_FLAGS "JUE"

#define BOOTSECTOR_HACK_VGA_OFFSET 0x3D
#define BOOTSECTOR_VGA_FLAG "1"

#define BOOTSECTOR_HACK_OS_OFFSET 0x3E
#define BOOTSECTOR_OS_FLAG "0"

#define BOOTSECTOR_HACK_BOOTSIZE_OFFSET 0x639C

#define BOOTSECTOR_HACK_OFFSET 0x3704

// The bootsector_hack_data payload and its size (BOOTSECTOR_HACK_DATA_SIZE)
// live in ip_data.hpp (included only by binhack.cpp) to keep this header
// readable.

#endif // __BINHACK__HPP__
