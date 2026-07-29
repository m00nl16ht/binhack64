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
         << "  " << progname << " hack0 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK0 (kikuchan): replaces every raw old-lba reference with" << endl
         << "      lba directly (no +166/+150 offset) in the boot binary." << endl
         << endl
         << "  " << progname << " hack <boot.bin> <lba> [old-lba]" << endl
         << "      HACK (Bero): replaces every (old-lba+166) reference with" << endl
         << "      (lba+166) in the boot binary." << endl
         << endl
         << "  " << progname << " hack2 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK2 (Unknown): replaces every (old-lba+150) reference with" << endl
         << "      (lba+150) in the boot binary." << endl
         << endl
         << "  " << progname << " hack3 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK3 (Pekearai): HACK + HACK2 combined." << endl
         << endl
         << "  " << progname << " dahack <boot.bin> <lba> [old-lba]" << endl
         << "      DAHACK (Mr. KiMWU): HACK(lba) + HACK2(0)." << endl
         << endl
         << "  " << progname << " cdda <boot.bin>" << endl
         << "      CDDA fix (Mr. KiMWU): fixes multi-track CDDA bootbins where" << endl
         << "      the first audio track reads as track04 instead of track01." << endl
         << endl
         << "  " << progname << " help" << endl
         << "      Show this help." << endl
         << endl
         << "  " << progname << " --version" << endl
         << "      Show version information." << endl
         << endl
         << "Notes:" << endl
         << "  - <lba> is the MSINFO/LBA value of the boot binary on the target" << endl
         << "    disc image. It is ignored for Windows CE binaries." << endl
         << "  - [old-lba] defaults to " << HACK_DEFAULT_OLD_LBA << " (how most original discs" << endl
         << "    were mastered before being re-burned at a new LBA)." << endl
         << "  - hack0/hack/hack2/hack3/dahack/cdda only touch the boot binary, never IP.BIN." << endl;
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

    if (command == "hack0" || command == "hack" || command == "hack2" ||
        command == "hack3" || command == "dahack") {
        if (argc != 4 && argc != 5) {
            cout << "Usage: " << argv[0] << " " << command << " <boot.bin> <lba> [old-lba]" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        unsigned int lba = parseLba(argv[3]);
        unsigned int oldLba = (argc == 5) ? parseLba(argv[4]) : HACK_DEFAULT_OLD_LBA;
        if (command == "hack0") return runHack0(argv[2], lba, oldLba);
        if (command == "hack") return runHack(argv[2], lba, oldLba);
        if (command == "hack2") return runHack2(argv[2], lba, oldLba);
        if (command == "hack3") return runHack3(argv[2], lba, oldLba);
        return runDahack(argv[2], lba, oldLba);
    }

    if (command == "cdda") {
        if (argc != 3) {
            cout << "Usage: " << argv[0] << " cdda <boot.bin>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return runCdda(argv[2]);
    }

    cout << "Unknown command: " << command << endl << endl;
    printUsage(argv[0]);
    return ExitCode::UsageError;
}
