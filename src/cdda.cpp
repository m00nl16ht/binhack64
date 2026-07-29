/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	CDDA fix patch engine implementation.
*/

#include "cdda.hpp"
#include "cdda_routine.hpp"

using namespace std;

bool applyCdda(fstream& boot, unsigned int bootsize) {

    // Locate the first CD001 signature (searchHackOffsets returns index-8
    // for every occurrence; we only need the first one, like Python's find).
    vector<unsigned int> hackoffsets = searchHackOffsets(boot, bootsize);
    if (hackoffsets.empty()) {
        cout << "No CD001 signature found in binary!" << endl;
        return false;
    }
    unsigned int cd001Index = hackoffsets[0] + 8;

    if (cd001Index < 40 || cd001Index - 40 + 4 > bootsize) {
        cout << "File does not look like a valid CDDA bootbin "
             << "(CD001 too close to the start of the file)." << endl;
        return false;
    }

    // Read the marker value near CD001 the offset is computed from.
    unsigned char valBytes[4];
    boot.seekg(cd001Index - 40, ios::beg);
    boot.read(reinterpret_cast<char*>(valBytes), 4);
    unsigned int val = static_cast<unsigned int>(valBytes[0])
                      | (static_cast<unsigned int>(valBytes[1]) << 8)
                      | (static_cast<unsigned int>(valBytes[2]) << 16)
                      | (static_cast<unsigned int>(valBytes[3]) << 24);

    if (val < CDDA_OFFSET_MAGIC) {
        cout << "File does not look like a valid CDDA bootbin "
             << "(unexpected marker value near CD001)." << endl;
        return false;
    }
    unsigned int offset = val - CDDA_OFFSET_MAGIC;

    // The routine, then two trailing 4-byte writes, must fit in the file.
    if (offset > bootsize || bootsize - offset < CDDA_ROUTINE_SIZE + 8) {
        cout << "File does not look like a valid CDDA bootbin "
             << "(patch would run past the end of the file)." << endl;
        return false;
    }

    boot.seekp(offset, ios::beg);
    boot.write(reinterpret_cast<const char*>(cdda_routine), CDDA_ROUTINE_SIZE);

    unsigned int a = val - 402;
    unsigned int b = val + 268;
    boot.seekp(offset + CDDA_ROUTINE_SIZE, ios::beg);
    boot.write(reinterpret_cast<const char*>(&a), sizeof(a));
    boot.seekp(offset + CDDA_ROUTINE_SIZE + 4, ios::beg);
    boot.write(reinterpret_cast<const char*>(&b), sizeof(b));

    cout << "CDDA fix applied at offset " << offset << "." << endl;
    return true;
}
