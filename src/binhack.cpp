/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Low-level BINHACK patch engine implementation.
*/

#include "binhack.hpp"
#include "ip_data.hpp"
#include <string.h>

// -----------------------------------------------------------------------------

// Get length of file
int filesize( istream& f ) {
  int l;
  f.seekg ( 0, ios::end );
  l = f.tellg();
  f.seekg ( 0, ios::beg );
  return l;
}

// -----------------------------------------------------------------------------

// Return true if the boot file is WinCE
bool isWinCE( istream& boot, unsigned int hackoffset ) {
	char wince[ BOOT_HACK_WINCE_CHECK_SIZE ];

    boot.seekg( hackoffset, ios::beg );
    boot.read( wince, BOOT_HACK_WINCE_CHECK_SIZE );
    boot.seekg( 0, ios::beg );

	// if read wince = wincecheck_ref then it's WinCE
	return (memcmp( &wince, &wincecheck_ref, BOOT_HACK_WINCE_CHECK_SIZE ) == 0);
}

// -----------------------------------------------------------------------------

// Search the CD001 offset
vector<unsigned int> searchHackOffsets( istream& boot, unsigned int bootsize ) {
    vector<unsigned int> hackoffsets;
	char *bootbuf;
	char tmp[ BOOT_HACK_SIGNATURE_SIZE ]; // 5 bytes for "CD001", no null terminator

	// Loading the Boot file in memory
	bootbuf = new char[ bootsize ];
	boot.seekg( 0, ios::beg );
	boot.read ( bootbuf, bootsize);

	// Searching all CD001 signatures in memory
    for ( int i = 0 ; i < ( bootsize - BOOT_HACK_SIGNATURE_SIZE ) ; i++ ) {
		// Copying the memory zone that we want to compare
		memcpy( &tmp, &bootbuf[i], BOOT_HACK_SIGNATURE_SIZE );

		// Comparing the buffer with the "CD001" reference
		if ( memcmp( tmp, bootsign_ref, BOOT_HACK_SIGNATURE_SIZE ) == 0 ) {
			hackoffsets.push_back( (unsigned int) i - 8 );
		}
    }

	// Free the memory
	delete[] bootbuf;

	// returning all found values
	return hackoffsets;
}

// -----------------------------------------------------------------------------

// Return true if the boot file is bincon'd
bool isBincon( istream& boot ) {
	char bincon[ BOOT_HACK_BINCON_CHECK_SIZE ];

    boot.seekg( 0, ios::beg );
    boot.read( bincon, BOOT_HACK_BINCON_CHECK_SIZE );
    boot.seekg( 0, ios::beg );

	// if read bincon = binconcheck_ref then it's Bincon'd
	return (memcmp( &bincon, &binconcheck_ref, BOOT_HACK_BINCON_CHECK_SIZE ) == 0);
}

// -----------------------------------------------------------------------------

void hackBootStrap( ofstream& iphak, unsigned int bootsize, fstream& boot ) {
    // Set all region flags
    iphak.seekp( BOOTSECTOR_HACK_REGION_FLAGS_OFFSET, ios::beg );
    iphak << BOOTSECTOR_REGION_FLAGS;

	// Enable VGA
    iphak.seekp( BOOTSECTOR_HACK_VGA_OFFSET, ios::beg );
    iphak << BOOTSECTOR_VGA_FLAG;

    if (isBincon(boot)){
    iphak.seekp( BOOTSECTOR_HACK_OS_OFFSET, ios::beg );
    iphak << BOOTSECTOR_OS_FLAG;
        cout << "Bincon detected, OS flag set to 0." << endl;
    }

    // Now real bootstrap hacking, stuff to put in iphak!
    iphak.seekp( BOOTSECTOR_HACK_OFFSET, ios::beg );
    iphak.write( reinterpret_cast<const char*>(bootsector_hack_data), BOOTSECTOR_HACK_DATA_SIZE );

	// Writing the BOOT.BIN filesize in the IPBIN
    iphak.seekp( BOOTSECTOR_HACK_BOOTSIZE_OFFSET, ios::beg );
    iphak.write( (char *) &bootsize, sizeof( unsigned int ) );
}

// -----------------------------------------------------------------------------

bool hackKatanaBootBinary(fstream& boot, unsigned int hackoffset, unsigned int lba ) {
		if (hackoffset != -1) {

			// Writes new LBA to bootbin to right address
			boot.seekp ( hackoffset, ios::beg );
			boot.write( (char *) &lba, sizeof( unsigned int ) );
			return true;

		} else {

			// hack offset not found
			return false;
		}
}
