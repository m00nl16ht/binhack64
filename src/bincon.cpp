/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	bincon patch engine implementation.
*/

#include "bincon.hpp"
#include <cstring>

bool applyBincon(fstream& boot, unsigned int bootsize) {
    bool isWinCEBin = detectWinCE(boot, bootsize);
    bool isBinconedBin = isBincon(boot);
    bool isKatanaBin = !(isWinCEBin || isBinconedBin);

    if (isWinCEBin && !isBinconedBin) {
        if (bootsize <= BINCON_CHUNK_SIZE) {
            cout << "File is too small to bincon (must be larger than "
                 << BINCON_CHUNK_SIZE << " bytes)." << endl;
            return false;
        }

        char* buf = new char[bootsize];
        boot.seekg(0, ios::beg);
        boot.read(buf, bootsize);

        unsigned int middleLen = bootsize - BINCON_CHUNK_SIZE;
        char* newbuf = new char[bootsize];
        memcpy(newbuf, buf + BINCON_CHUNK_SIZE, middleLen);
        memcpy(newbuf + middleLen, buf + middleLen, BINCON_CHUNK_SIZE);

        boot.seekp(0, ios::beg);
        boot.write(newbuf, bootsize);

        delete[] buf;
        delete[] newbuf;

        cout << "File successfully bincon'd." << endl;
        return true;
    }

    if (isKatanaBin) {
        cout << "Error: bootbin detected as a Katana binary." << endl
             << "       No binconing required." << endl;
        return false;
    }
    if (isWinCEBin && isBinconedBin) {
        cout << "Error: the bootbin is already bincon'd." << endl;
        return false;
    }
    // !isWinCEBin && isBinconedBin
    cout << "Error: bootbin detected as bincon'd but not WinCE." << endl
         << "       This should not be." << endl;
    return false;
}
