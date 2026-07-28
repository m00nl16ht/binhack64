/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Command-line front-end implementation.
*/

#include "cli.hpp"
#include "commands.hpp"
#include <cstdlib>

namespace {

const char* BANNER = "IP.BIN/BOOT.BIN Selfboot Hacker - binhack64";

void printVersion() {
    cout << BANNER << endl
         << "Modernized 64-bit BINHACK clone for the Sega Dreamcast." << endl
         << "Copyright (C) FamilyGuy and contributors." << endl
         << "Licensed under the GNU General Public License v3 (GPLv3)." << endl;
}

void printUsage(const char* progname) {
    cout << BANNER << endl << endl
         << "Usage:" << endl
         << "  " << progname << endl
         << "      Interactive mode (prompts for input, patches both files)" << endl
         << endl
         << "  " << progname << " patch-boot <boot.bin> <lba>" << endl
         << "      Patch BOOT.BIN (1ST_READ.BIN) only: writes the LBA hack." << endl
         << endl
         << "  " << progname << " patch-ip <boot.bin> <ip.bin>" << endl
         << "      Patch IP.BIN only: writes region/VGA/bincon flags and the" << endl
         << "      BOOT.BIN size. The unpatched template is always read from" << endl
         << "      \"" << BOOTSECTOR_NAME << "\" in the current directory; <ip.bin>" << endl
         << "      is the output filename." << endl
         << endl
         << "  " << progname << " patch-all <boot.bin> <ip.bin> <lba>" << endl
         << "      Patch both BOOT.BIN and IP.BIN (classic BINHACK behavior)." << endl
         << endl
         << "  " << progname << " help" << endl
         << "      Show this help." << endl
         << endl
         << "  " << progname << " --version" << endl
         << "      Show version information." << endl
         << endl
         << "Notes:" << endl
         << "  - <lba> is the MSINFO/LBA value of the boot binary on the target" << endl
         << "    disc image. It is ignored for Windows CE binaries." << endl;
}

unsigned int parseLba(const char* value) {
    return static_cast<unsigned int>(atoi(value));
}

// Reproduces the original interactive prompt sequence: binary name, then
// bootsector name, then (only if not WinCE) the msinfo/LBA value. Uses the
// mid-level building blocks directly, since patchAll() alone can't ask for
// the LBA only after WinCE detection.
int runInteractive() {
    fstream boot;
    unsigned int bootsize;
    vector<unsigned int> hackoffsets;
    unsigned int lba = 0;
    string bootname, ipname;
    char iphackbuf[BOOTSECTOR_SIZE];
    bool isWinCEBinary = false;

    cout << BANNER << endl << endl;

    cout << "Enter name of binary: ";
    cin >> bootname;

    cout << "Enter name of bootsector: ";
    cin >> ipname;

    if (!processBootBin(bootname, bootsize, hackoffsets, boot)) {
        return ExitCode::BootOpenError;
    }

    if (!isWinCE(boot, hackoffsets[0])) {
        cout << "Enter msinfo value: ";
        cin >> lba;
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

} // namespace

int runCli(int argc, char* argv[]) {
    if (argc == 1) {
        return runInteractive();
    }

    string command = argv[1];

    if (command == "--version" || command == "-v") {
        printVersion();
        return ExitCode::Ok;
    }

    if (command == "help" || command == "--help" || command == "-h") {
        printUsage(argv[0]);
        return ExitCode::Ok;
    }

    if (command == "patch-boot") {
        if (argc != 4) {
            cout << "Usage: " << argv[0] << " patch-boot <boot.bin> <lba>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return patchBoot(argv[2], parseLba(argv[3]));
    }

    if (command == "patch-ip") {
        if (argc != 4) {
            cout << "Usage: " << argv[0] << " patch-ip <boot.bin> <ip.bin>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return patchIp(argv[2], argv[3]);
    }

    if (command == "patch-all") {
        if (argc != 5) {
            cout << "Usage: " << argv[0] << " patch-all <boot.bin> <ip.bin> <lba>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return patchAll(argv[2], argv[3], parseLba(argv[4]));
    }

    cout << "Unknown command: " << command << endl << endl;
    printUsage(argv[0]);
    return ExitCode::UsageError;
}
