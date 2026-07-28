/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	BINHACK patch commands implementation.
*/

#include "commands.hpp"

// ============================================================================
// MID-LEVEL BUILDING BLOCKS
// ============================================================================

bool processBootBin(const string& bootname, unsigned int& bootsize,
                    vector<unsigned int>& hackoffsets, fstream& boot) {

    // Opening the BOOT.BIN
    boot.open(bootname.c_str(), ios::binary | ios::in | ios::out);
    if (boot.fail()) {
        cout << "Error opening " << bootname << "." << endl;
        return false;
    }

    // Get the BOOT.BIN filesize
    bootsize = filesize(boot);

    // Getting all CD001 offsets
    hackoffsets = searchHackOffsets(boot, bootsize);

#ifdef DEBUG
    cout << "[DEBUG] filename: " << bootname.c_str() << endl
         << "[DEBUG] opened: " << boot.is_open() << endl
         << "[DEBUG] boot.bin filesize= " << bootsize << endl
         << "[DEBUG] Found " << hackoffsets.size() << " CD001 signature(s)" << endl;
    for (size_t i = 0; i < hackoffsets.size(); i++) {
        cout << "[DEBUG] Hack Offset " << i << ": " << hackoffsets[i] << endl;
    }
#endif

    if (hackoffsets.empty()) {
        cout << "No CD001 signature found in binary!" << endl;
        return false;
    }

    return true;
}

bool hackBootBin(fstream& boot, const vector<unsigned int>& hackoffsets,
                 unsigned int lba, bool& isWinCEBinary) {

    // Use the first offset for WinCE detection
    if (isWinCE(boot, hackoffsets[0])) {
        // WinCE Executable
        cout << "Found Windows CE" << endl;
        isWinCEBinary = true;
        // Original binhack doesn't modify WinCE binary
        // Only need to bincon it and disable WinCEOS flag in ip.bin
        return true;

    } else {
        // Katana Executable
        isWinCEBinary = false;

        // Changes lba to lba+166 to get the real number to hack the bootbin
        lba += 166;

        // Hacking the Katana Binary at all found offsets
        bool allSuccess = true;
        for (size_t i = 0; i < hackoffsets.size(); i++) {
            cout << "Hacking offset " << i << " at position " << hackoffsets[i] << "..." << endl;
            if (!hackKatanaBootBinary(boot, hackoffsets[i], lba)) {
                cout << "Failed to hack offset " << i << endl;
                allSuccess = false;
            }
        }

        if (allSuccess) {
            cout << "File successfully hacked (" << hackoffsets.size()
                 << " location(s) patched)." << endl;
            return true;
        } else {
            cout << "Invalid binary file or partial hack failure." << endl;
            return false;
        }
    }
}

bool loadIpBin(char* buffer) {

    ifstream ipbin;

    // Opening the IP.BIN
    ipbin.open(BOOTSECTOR_NAME, ios::in);
    if (ipbin.fail()) {
        cout << "Error opening ip file." << endl;
        return false;
    }

    // Read ip.bin content
    ipbin.read(buffer, BOOTSECTOR_SIZE);
    ipbin.close();

    return true;
}

bool createHackedIpBin(const string& ipname, char* iphackbuf,
                       unsigned int bootsize, fstream& boot) {

    ofstream iphak;

    // Creating the IP.HAK bootsector file
    iphak.open(ipname.c_str(), ios::trunc | ios::out | ios::binary);
    if (iphak.fail()) {
        cout << "Error creating " << ipname << "." << endl;
        return false;
    }

    // Writing the IP.HAK with the original content of the IP.BIN
    iphak.write(iphackbuf, BOOTSECTOR_SIZE);

    // Hack the bootstrap
    hackBootStrap(iphak, bootsize, boot);

    // Finishing...
    iphak.close();
    cout << "File " << ipname << " successfully created." << endl;

    return true;
}

// ============================================================================
// HIGH-LEVEL COMMANDS
// ============================================================================

int patchBoot(const string& bootname, unsigned int lba) {
    fstream boot;
    unsigned int bootsize;
    vector<unsigned int> hackoffsets;
    bool isWinCEBinary = false;

    if (!processBootBin(bootname, bootsize, hackoffsets, boot)) {
        return ExitCode::BootOpenError;
    }

    int result = ExitCode::Ok;
    if (!hackBootBin(boot, hackoffsets, lba, isWinCEBinary)) {
        result = ExitCode::BootHackError;
    }

    boot.close();
    return result;
}

int patchIp(const string& bootname, const string& ipname) {
    fstream boot;
    char iphackbuf[BOOTSECTOR_SIZE];

    // Opened read-only: patch-ip only needs the boot binary's size and its
    // bincon status, so BOOT.BIN itself is never modified here.
    boot.open(bootname.c_str(), ios::binary | ios::in);
    if (boot.fail()) {
        cout << "Error opening " << bootname << "." << endl;
        return ExitCode::BootOpenError;
    }

    unsigned int bootsize = filesize(boot);

    int result = ExitCode::Ok;
    if (!loadIpBin(iphackbuf)) {
        result = ExitCode::IpOpenError;
    } else if (!createHackedIpBin(ipname, iphackbuf, bootsize, boot)) {
        result = ExitCode::IpWriteError;
    }

    boot.close();
    return result;
}

int patchAll(const string& bootname, const string& ipname, unsigned int lba) {
    fstream boot;
    unsigned int bootsize;
    vector<unsigned int> hackoffsets;
    char iphackbuf[BOOTSECTOR_SIZE];
    bool isWinCEBinary = false;

    if (!processBootBin(bootname, bootsize, hackoffsets, boot)) {
        return ExitCode::BootOpenError;
    }

    if (!hackBootBin(boot, hackoffsets, lba, isWinCEBinary)) {
        boot.close();
        return ExitCode::BootHackError;
    }

    int result = ExitCode::Ok;
    if (!loadIpBin(iphackbuf)) {
        result = ExitCode::IpOpenError;
    } else if (!createHackedIpBin(ipname, iphackbuf, bootsize, boot)) {
        result = ExitCode::IpWriteError;
    }

    boot.close();
    return result;
}
