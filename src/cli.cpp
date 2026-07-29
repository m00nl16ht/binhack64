/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Command-line front-end implementation.
*/

#include "cli.hpp"
#include "commands.hpp"
#include <cstdlib>

namespace {

const char* BANNER = "IP.BIN/BOOT.BIN Selfboot Hacker - binhack64";

// Set once at the top of runCli() from argv[0]'s basename, so usage text
// reflects the actual invoked filename rather than a hardcoded name or a
// full path.
string PROGNAME = "binhack64";

string basename(const string& path) {
    size_t pos = path.find_last_of("/\\");
    return (pos == string::npos) ? path : path.substr(pos + 1);
}

void printVersion() {
    cout << BANNER << endl
         << "Modernized 64-bit BINHACK clone for the Sega Dreamcast." << endl
         << "Copyright (C) 2010-2026 FamilyGuy and contributors." << endl
         << "Licensed under the GNU General Public License v3 (GPLv3)." << endl;
}

void printUsage() {
    cout << BANNER << endl << endl
         << "Usage:" << endl
         << "  " << PROGNAME << endl
         << "      Interactive mode (prompts for input, patches both files)" << endl
         << endl
         << "  " << PROGNAME << " binhack-boot <boot.bin> <lba>" << endl
         << "      Patch BOOT.BIN (1ST_READ.BIN) only: writes the LBA hack." << endl
         << endl
         << "  " << PROGNAME << " binhack-ip <boot.bin> <ip.bin>" << endl
         << "      Patch IP.BIN only: writes region/VGA/bincon flags and the" << endl
         << "      BOOT.BIN size. The unpatched template is always read from" << endl
         << "      \"" << BOOTSECTOR_NAME << "\" in the current directory; <ip.bin>" << endl
         << "      is the output filename." << endl
         << endl
         << "  " << PROGNAME << " binhack <boot.bin> <ip.bin> <lba>" << endl
         << "      Patch both BOOT.BIN and IP.BIN (classic BINHACK behavior)." << endl
         << endl
         << "  " << PROGNAME << " hack0 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK0 (kikuchan): replaces every raw old-lba reference with" << endl
         << "      lba directly (no +166/+150 offset) in the boot binary." << endl
         << endl
         << "  " << PROGNAME << " hack|hack1 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK1 (Bero): replaces every (old-lba+166) reference with" << endl
         << "      (lba+166) in the boot binary." << endl
         << endl
         << "  " << PROGNAME << " hack2 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK2 (Unknown): replaces every (old-lba+150) reference with" << endl
         << "      (lba+150) in the boot binary." << endl
         << endl
         << "  " << PROGNAME << " hack3 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK3 (Pekearai): HACK1 + HACK2 combined." << endl
         << endl
         << "  " << PROGNAME << " dahack <boot.bin> <lba> [old-lba]" << endl
         << "      DAHACK (Mr. KiMWU): HACK1(lba) + HACK2(0)." << endl
         << endl
         << "  " << PROGNAME << " cdda <boot.bin>" << endl
         << "      CDDA fix (Mr. KiMWU): fixes multi-track CDDA bootbins where" << endl
         << "      the first audio track reads as track04 instead of track01." << endl
         << endl
         << "  " << PROGNAME << " bincon <boot.bin>" << endl
         << "      bincon (dopefish/echelon): voodoo to make a raw WinCE boot" << endl
         << "      binary bootable. Run binhack-ip afterward to also set IP.BIN's" << endl
         << "      OS flag - it detects bincon'd binaries automatically." << endl
         << endl
         << "  " << PROGNAME << " unprotect <boot.bin> <id>" << endl
         << "      Removes one of 7 non-LBA copy protections. <id> is 0-6 or" << endl
         << "      \"jsr\" (alias for 6). See README.md for the full credited list." << endl
         << endl
         << "  " << PROGNAME << " check-protection <boot.bin> [id]" << endl
         << "      Read-only: reports whether <id>'s original and/or cracked" << endl
         << "      pattern is present, without modifying the file. Omit <id> to" << endl
         << "      scan all 7." << endl
         << endl
         << "  " << PROGNAME << " wince-cdda-fix-ip <ip.bin>" << endl
         << "      WinCE+CDDA fix (pitito): fixes IP.BIN so CDDA audio doesn't" << endl
         << "      break when converting a WinCE game from GDI to CDI. Unlike" << endl
         << "      every other command above, this patches IP.BIN itself - run" << endl
         << "      it after binhack-ip/binhack, not instead of." << endl
         << endl
         << "  " << PROGNAME << " help" << endl
         << "      Show this help (--help, -h)." << endl
         << endl
         << "  " << PROGNAME << " version" << endl
         << "      Show version information (--version, -v)." << endl
         << endl
         << "Notes:" << endl
         << "  - <lba> is the MSINFO/LBA value of the boot binary on the target" << endl
         << "    disc image. It is ignored for Windows CE binaries." << endl
         << "  - [old-lba] defaults to " << HACK_DEFAULT_OLD_LBA << " (how most original discs" << endl
         << "    were mastered before being re-burned at a new LBA)." << endl
         << "  - hack0/hack/hack2/hack3/dahack/cdda/bincon/unprotect only touch the" << endl
         << "    boot binary, never IP.BIN. wince-cdda-fix-ip is the opposite: it" << endl
         << "    only touches IP.BIN, never the boot binary." << endl;
}

unsigned int parseLba(const char* value) {
    return static_cast<unsigned int>(atoi(value));
}

// Reproduces the original interactive prompt sequence: binary name, then
// bootsector name, then (only if not WinCE) the msinfo/LBA value. Uses the
// mid-level building blocks directly, since runBinhack() can't defer the
// LBA prompt until after WinCE detection.
int runInteractive() {
    fstream boot;
    unsigned int bootsize;
    vector<unsigned int> hackoffsets;
    unsigned int lba = 0;
    string bootname, ipname;
    char iphackbuf[BOOTSECTOR_SIZE];
    bool isWinCEBinary = false;

    cout << BANNER << endl
         << "Interactive mode (classic BINHACK trick). For other patching" << endl
         << "methods, run: " << PROGNAME << " --help" << endl
         << endl;

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
    PROGNAME = basename(argv[0]);

    if (argc == 1) {
        return runInteractive();
    }

    string command = argv[1];

    if (command == "version" || command == "--version" || command == "-v") {
        printVersion();
        return ExitCode::Ok;
    }

    if (command == "help" || command == "--help" || command == "-h") {
        printUsage();
        return ExitCode::Ok;
    }

    if (command == "binhack-boot") {
        if (argc != 4) {
            cout << "Usage: " << PROGNAME << " binhack-boot <boot.bin> <lba>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return runBinhackBoot(argv[2], parseLba(argv[3]));
    }

    if (command == "binhack-ip") {
        if (argc != 4) {
            cout << "Usage: " << PROGNAME << " binhack-ip <boot.bin> <ip.bin>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return runBinhackIp(argv[2], argv[3]);
    }

    if (command == "binhack") {
        if (argc != 5) {
            cout << "Usage: " << PROGNAME << " binhack <boot.bin> <ip.bin> <lba>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return runBinhack(argv[2], argv[3], parseLba(argv[4]));
    }

    if (command == "hack0" || command == "hack" || command == "hack1" ||
        command == "hack2" || command == "hack3" || command == "dahack") {
        if (argc != 4 && argc != 5) {
            cout << "Usage: " << PROGNAME << " " << command << " <boot.bin> <lba> [old-lba]" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        unsigned int lba = parseLba(argv[3]);
        unsigned int oldLba = (argc == 5) ? parseLba(argv[4]) : HACK_DEFAULT_OLD_LBA;
        if (command == "hack0") return runHack0(argv[2], lba, oldLba);
        if (command == "hack" || command == "hack1") return runHack(argv[2], lba, oldLba);
        if (command == "hack2") return runHack2(argv[2], lba, oldLba);
        if (command == "hack3") return runHack3(argv[2], lba, oldLba);
        return runDahack(argv[2], lba, oldLba);
    }

    if (command == "cdda") {
        if (argc != 3) {
            cout << "Usage: " << PROGNAME << " cdda <boot.bin>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return runCdda(argv[2]);
    }

    if (command == "bincon") {
        if (argc != 3) {
            cout << "Usage: " << PROGNAME << " bincon <boot.bin>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return runBincon(argv[2]);
    }

    if (command == "unprotect") {
        if (argc != 4) {
            cout << "Usage: " << PROGNAME << " unprotect <boot.bin> <id>" << endl;
            return ExitCode::UsageError;
        }
        int variant = unprotectVariantFromString(argv[3]);
        if (variant == -1) {
            cout << "Invalid <id>: must be 0-6 or \"jsr\"." << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return runUnprotect(argv[2], variant);
    }

    if (command == "check-protection") {
        if (argc != 3 && argc != 4) {
            cout << "Usage: " << PROGNAME << " check-protection <boot.bin> [id]" << endl;
            return ExitCode::UsageError;
        }
        int variant = -1;
        if (argc == 4) {
            variant = unprotectVariantFromString(argv[3]);
            if (variant == -1) {
                cout << "Invalid <id>: must be 0-6 or \"jsr\"." << endl;
                return ExitCode::UsageError;
            }
        }
        cout << BANNER << endl << endl;
        return runCheckProtection(argv[2], variant);
    }

    if (command == "wince-cdda-fix-ip") {
        if (argc != 3) {
            cout << "Usage: " << PROGNAME << " wince-cdda-fix-ip <ip.bin>" << endl;
            return ExitCode::UsageError;
        }
        cout << BANNER << endl << endl;
        return runWinceCddaFix(argv[2]);
    }

    cout << "Unknown command: " << command << endl << endl;
    printUsage();
    return ExitCode::UsageError;
}
