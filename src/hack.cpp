/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	HACK / HACK2 / HACK3 / DAHACK patch engine implementation.
*/

#include "hack.hpp"
#include <cstring>

unsigned int searchReplaceBytes(fstream& boot, unsigned int bootsize,
                                const unsigned char* oldPattern, const unsigned char* newPattern,
                                unsigned int patternLen) {
    if (patternLen == 0 || bootsize < patternLen) {
        return 0;
    }

    // Loading the boot file in memory to search it
    char* bootbuf = new char[bootsize];
    boot.seekg(0, ios::beg);
    boot.read(bootbuf, bootsize);

    // Non-overlapping left-to-right scan: a match consumes patternLen
    // bytes before the scan resumes.
    unsigned int replacements = 0;
    unsigned int i = 0;
    while (i + patternLen <= bootsize) {
        if (memcmp(&bootbuf[i], oldPattern, patternLen) == 0) {
            boot.seekp(i, ios::beg);
            boot.write(reinterpret_cast<const char*>(newPattern), patternLen);
            replacements++;
            i += patternLen;
        } else {
            i++;
        }
    }

    delete[] bootbuf;
    return replacements;
}

bool containsBytes(istream& boot, unsigned int bootsize,
                   const unsigned char* pattern, unsigned int patternLen) {
    if (patternLen == 0 || bootsize < patternLen) {
        return false;
    }

    char* bootbuf = new char[bootsize];
    boot.seekg(0, ios::beg);
    boot.read(bootbuf, bootsize);

    bool found = false;
    for (unsigned int i = 0; i + patternLen <= bootsize; i++) {
        if (memcmp(&bootbuf[i], pattern, patternLen) == 0) {
            found = true;
            break;
        }
    }

    delete[] bootbuf;
    return found;
}

unsigned int searchReplaceUint32LE(fstream& boot, unsigned int bootsize,
                                    unsigned int oldValue, unsigned int newValue) {
    unsigned char oldBytes[4], newBytes[4];
    for (int i = 0; i < 4; i++) {
        oldBytes[i] = static_cast<unsigned char>((oldValue >> (i * 8)) & 0xFF);
        newBytes[i] = static_cast<unsigned char>((newValue >> (i * 8)) & 0xFF);
    }
    return searchReplaceBytes(boot, bootsize, oldBytes, newBytes, 4);
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
