/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	BINHACK patch commands implementation.
*/

#include "commands.hpp"

using namespace std;

// Opens bootname read/write and gets its size, without requiring a CD001
// signature (unlike processBootBin) - hack/hack2/hack3/dahack/cdda/bincon
// don't all need one.
static bool openBootReadWrite(const string& bootname, unsigned int& bootsize, fstream& boot) {
    boot.open(bootname.c_str(), ios::binary | ios::in | ios::out);
    if (boot.fail()) {
        cout << "Error opening " << bootname << "." << endl;
        return false;
    }
    bootsize = filesize(boot);
    return true;
}

// Opens bootname read-only and gets its size, for commands that only
// ever inspect the boot binary (binhack-ip, check-protection).
static bool openBootReadOnly(const string& bootname, unsigned int& bootsize, fstream& boot) {
    boot.open(bootname.c_str(), ios::binary | ios::in);
    if (boot.fail()) {
        cout << "Error opening " << bootname << "." << endl;
        return false;
    }
    bootsize = filesize(boot);
    return true;
}

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

bool loadIpBin(const string& ipname, char* buffer) {

    ifstream ipbin;

    // Opening the IP.BIN (binary: without it, Windows text mode eats CR
    // bytes and shifts the rest of the bootsector)
    ipbin.open(ipname.c_str(), ios::in | ios::binary);
    if (ipbin.fail()) {
        cout << "Error opening " << ipname << "." << endl;
        return false;
    }

    // Read ip.bin content
    ipbin.read(buffer, BOOTSECTOR_SIZE);
    if (ipbin.gcount() != BOOTSECTOR_SIZE) {
        cout << "Error: " << ipname << " is not a full " << BOOTSECTOR_SIZE
             << "-byte bootsector." << endl;
        ipbin.close();
        return false;
    }
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
    cout << "File " << ipname << " successfully patched." << endl;

    return true;
}

// ============================================================================
// HIGH-LEVEL COMMANDS
// ============================================================================

int runBinhackBoot(const string& bootname, unsigned int lba) {
    fstream boot;
    unsigned int bootsize;
    vector<unsigned int> hackoffsets;
    bool isWinCEBinary = false;

    if (!ensureBackup(bootname)) {
        return ExitCode::BackupError;
    }

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

int runBinhackIp(const string& bootname, const string& ipname) {
    fstream boot;
    unsigned int bootsize;
    char iphackbuf[BOOTSECTOR_SIZE];

    if (!ensureBackup(ipname)) {
        return ExitCode::BackupError;
    }

    // Opened read-only: binhack-ip only needs the boot binary's size and its
    // bincon status, so BOOT.BIN itself is never modified here.
    if (!openBootReadOnly(bootname, bootsize, boot)) {
        return ExitCode::BootOpenError;
    }

    int result = ExitCode::Ok;
    if (!loadIpBin(ipname, iphackbuf)) {
        result = ExitCode::IpOpenError;
    } else if (!createHackedIpBin(ipname, iphackbuf, bootsize, boot)) {
        result = ExitCode::IpWriteError;
    }

    boot.close();
    return result;
}

int runBinhack(const string& bootname, const string& ipname, unsigned int lba) {
    fstream boot;
    unsigned int bootsize;
    vector<unsigned int> hackoffsets;
    char iphackbuf[BOOTSECTOR_SIZE];
    bool isWinCEBinary = false;

    // Both files are modified, so back both up before touching either.
    if (!ensureBackup(bootname) || !ensureBackup(ipname)) {
        return ExitCode::BackupError;
    }

    if (!processBootBin(bootname, bootsize, hackoffsets, boot)) {
        return ExitCode::BootOpenError;
    }

    if (!hackBootBin(boot, hackoffsets, lba, isWinCEBinary)) {
        boot.close();
        return ExitCode::BootHackError;
    }

    int result = ExitCode::Ok;
    if (!loadIpBin(ipname, iphackbuf)) {
        result = ExitCode::IpOpenError;
    } else if (!createHackedIpBin(ipname, iphackbuf, bootsize, boot)) {
        result = ExitCode::IpWriteError;
    }

    boot.close();
    return result;
}

// ============================================================================
// HACK / HACK2 / HACK3 / DAHACK / CDDA COMMANDS
// ============================================================================

static int runNumericHack(const string& bootname, unsigned int lba, unsigned int oldLba,
                          unsigned int (*apply)(fstream&, unsigned int, unsigned int, unsigned int),
                          const char* label) {
    fstream boot;
    unsigned int bootsize;

    if (!ensureBackup(bootname)) {
        return ExitCode::BackupError;
    }

    if (!openBootReadWrite(bootname, bootsize, boot)) {
        return ExitCode::BootOpenError;
    }

    unsigned int count = apply(boot, bootsize, lba, oldLba);
    boot.close();

    cout << label << ": " << count << " location(s) patched." << endl;
    return ExitCode::Ok;
}

int runHack0(const string& bootname, unsigned int lba, unsigned int oldLba) {
    return runNumericHack(bootname, lba, oldLba, applyHack0, "HACK0");
}

int runHack(const string& bootname, unsigned int lba, unsigned int oldLba) {
    return runNumericHack(bootname, lba, oldLba, applyHack, "HACK");
}

int runHack2(const string& bootname, unsigned int lba, unsigned int oldLba) {
    return runNumericHack(bootname, lba, oldLba, applyHack2, "HACK2");
}

int runHack3(const string& bootname, unsigned int lba, unsigned int oldLba) {
    return runNumericHack(bootname, lba, oldLba, applyHack3, "HACK3");
}

int runDahack(const string& bootname, unsigned int lba, unsigned int oldLba) {
    return runNumericHack(bootname, lba, oldLba, applyDahack, "DAHACK");
}

int runCdda(const string& bootname) {
    fstream boot;
    unsigned int bootsize;

    if (!ensureBackup(bootname)) {
        return ExitCode::BackupError;
    }

    if (!openBootReadWrite(bootname, bootsize, boot)) {
        return ExitCode::BootOpenError;
    }

    int result = ExitCode::Ok;
    if (!applyCdda(boot, bootsize)) {
        result = ExitCode::BootHackError;
    }

    boot.close();
    return result;
}

int runBincon(const string& bootname) {
    fstream boot;
    unsigned int bootsize;

    if (!ensureBackup(bootname)) {
        return ExitCode::BackupError;
    }

    if (!openBootReadWrite(bootname, bootsize, boot)) {
        return ExitCode::BootOpenError;
    }

    int result = ExitCode::Ok;
    if (!applyBincon(boot, bootsize)) {
        result = ExitCode::BootHackError;
    }

    boot.close();
    return result;
}

int runUnprotect(const string& bootname, int variant) {
    fstream boot;
    unsigned int bootsize;

    if (!ensureBackup(bootname)) {
        return ExitCode::BackupError;
    }

    if (!openBootReadWrite(bootname, bootsize, boot)) {
        return ExitCode::BootOpenError;
    }

    unsigned int count = 0;
    bool valid = applyUnprotect(boot, bootsize, variant, count);
    boot.close();

    if (!valid) {
        cout << "Invalid unprotect variant." << endl;
        return ExitCode::UsageError;
    }

    cout << "unprotect " << variant << " (" << unprotectVariantCredit(variant)
         << "): " << count << " location(s) patched." << endl;
    return ExitCode::Ok;
}

static void printProtectionStatus(istream& boot, unsigned int bootsize, int variant) {
    bool protectedFound = false, unprotectedFound = false;
    checkProtection(boot, bootsize, variant, protectedFound, unprotectedFound);
    cout << "  " << variant << " (" << unprotectVariantCredit(variant) << "): "
         << "original pattern " << (protectedFound ? "found" : "not found") << ", "
         << "cracked pattern " << (unprotectedFound ? "found" : "not found") << endl;
}

int runCheckProtection(const string& bootname, int variant) {
    fstream boot;
    unsigned int bootsize;

    if (!openBootReadOnly(bootname, bootsize, boot)) {
        return ExitCode::BootOpenError;
    }

    if (variant == -1) {
        cout << "Protection scan for " << bootname << ":" << endl;
        for (int i = 0; i < UNPROTECT_VARIANT_COUNT; i++) {
            printProtectionStatus(boot, bootsize, i);
        }
        boot.close();
        return ExitCode::Ok;
    }

    if (variant < 0 || variant >= UNPROTECT_VARIANT_COUNT) {
        boot.close();
        cout << "Invalid unprotect variant." << endl;
        return ExitCode::UsageError;
    }

    printProtectionStatus(boot, bootsize, variant);
    boot.close();
    return ExitCode::Ok;
}

int runWinceCddaFix(const string& ipname) {
    fstream ip;
    unsigned int ipsize;

    if (!ensureBackup(ipname)) {
        return ExitCode::BackupError;
    }

    if (!openBootReadWrite(ipname, ipsize, ip)) {
        return ExitCode::IpOpenError;
    }

    WinceCddaFixResult result = applyWinceCddaFix(ip, ipsize);
    ip.close();

    switch (result) {
        case WinceCddaFixResult::Applied:
            cout << "WinCE+CDDA fix applied to " << ipname << "." << endl;
            return ExitCode::Ok;
        case WinceCddaFixResult::AlreadyApplied:
            cout << ipname << " already has the WinCE+CDDA fix applied." << endl;
            return ExitCode::Ok;
        default:
            cout << "File does not look like a binhack-patched IP.BIN "
                 << "(unexpected bytes at the fix offset)." << endl;
            return ExitCode::IpWriteError;
    }
}
