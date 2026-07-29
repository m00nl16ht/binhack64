/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	HACK / HACK2 / HACK3 / DAHACK patch engine implementation.
*/

#include "hack.hpp"
#include <cstring>

unsigned int searchReplaceUint32LE(fstream& boot, unsigned int bootsize,
                                    unsigned int oldValue, unsigned int newValue) {
    if (bootsize < 4) {
        return 0;
    }

    unsigned char oldBytes[4], newBytes[4];
    for (int i = 0; i < 4; i++) {
        oldBytes[i] = static_cast<unsigned char>((oldValue >> (i * 8)) & 0xFF);
        newBytes[i] = static_cast<unsigned char>((newValue >> (i * 8)) & 0xFF);
    }

    // Loading the boot file in memory to search it
    char* bootbuf = new char[bootsize];
    boot.seekg(0, ios::beg);
    boot.read(bootbuf, bootsize);

    // Non-overlapping left-to-right scan, matching Python's str.replace:
    // a match consumes 4 bytes before the scan resumes.
    unsigned int replacements = 0;
    unsigned int i = 0;
    while (i + 4 <= bootsize) {
        if (memcmp(&bootbuf[i], oldBytes, 4) == 0) {
            boot.seekp(i, ios::beg);
            boot.write(reinterpret_cast<const char*>(newBytes), 4);
            replacements++;
            i += 4;
        } else {
            i++;
        }
    }

    delete[] bootbuf;
    return replacements;
}

unsigned int applyHack0(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba) {
    return searchReplaceUint32LE(boot, bootsize, oldLba, lba);
}

unsigned int applyHack(fstream& boot, unsigned int bootsize,
                       unsigned int lba, unsigned int oldLba) {
    return searchReplaceUint32LE(boot, bootsize,
                                  oldLba + HACK_LBA_OFFSET, lba + HACK_LBA_OFFSET);
}

unsigned int applyHack2(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba) {
    return searchReplaceUint32LE(boot, bootsize,
                                  oldLba + HACK2_LBA_OFFSET, lba + HACK2_LBA_OFFSET);
}

unsigned int applyHack3(fstream& boot, unsigned int bootsize,
                        unsigned int lba, unsigned int oldLba) {
    unsigned int count = applyHack(boot, bootsize, lba, oldLba);
    count += applyHack2(boot, bootsize, lba, oldLba);
    return count;
}

unsigned int applyDahack(fstream& boot, unsigned int bootsize,
                         unsigned int lba, unsigned int oldLba) {
    unsigned int count = applyHack(boot, bootsize, lba, oldLba);
    count += applyHack2(boot, bootsize, 0, oldLba);
    return count;
}
