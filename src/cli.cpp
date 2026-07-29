/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Command-line front-end implementation.
*/

#include "cli.hpp"
#include "commands.hpp"
#include "version.hpp"
#include <cstdlib>
#include <cctype>

using namespace std;

namespace {

// Set once at the top of runCli() from argv[0]'s basename, so usage text
// reflects the actual invoked filename rather than a hardcoded name or a
// full path.
string PROGNAME = BINHACK64_NAME;

// argv[0]'s filename, without directories and without a trailing ".exe".
string basename(const string& path) {
    size_t pos = path.find_last_of("/\\");
    string name = (pos == string::npos) ? path : path.substr(pos + 1);

    const string ext = ".exe";
    if (name.size() > ext.size()) {
        string tail = name.substr(name.size() - ext.size());
        for (size_t i = 0; i < tail.size(); ++i) {
            tail[i] = static_cast<char>(tolower(static_cast<unsigned char>(tail[i])));
        }
        if (tail == ext) {
            name.erase(name.size() - ext.size());
        }
    }

    return name;
}

void printBanner() {
    cout << PROGNAME << " - v" << BINHACK64_VERSION << " - "
         << BINHACK64_DESCRIPTION << endl
         << BINHACK64_COPYRIGHT << endl
         << endl;
}

void printVersion() {
    printBanner();
    cout << BINHACK64_LICENSE << endl;
}

void printUsage() {
    printBanner();
    cout << "Usage:" << endl
         << "  " << PROGNAME << endl
         << "      Patch both BOOT.BIN (1ST_READ.BIN) and IP.BIN (classic binhack" << endl
         << "      behavior), in interactive mode: prompts for the filenames and the" << endl
         << "      LBA" << endl
         << endl
         << "  " << PROGNAME << " binhack-boot <boot.bin> <lba>" << endl
         << "      Patch BOOT.BIN (1ST_READ.BIN) only: writes the LBA hack" << endl
         << endl
         << "  " << PROGNAME << " binhack-ip <boot.bin> <ip.bin>" << endl
         << "      Patch IP.BIN only: writes region/VGA/bincon flags and the BOOT.BIN" << endl
         << "      size. <ip.bin> is patched in place" << endl
         << endl
         << "  " << PROGNAME << " binhack <boot.bin> <ip.bin> <lba>" << endl
         << "      Patch both BOOT.BIN and IP.BIN in place (classic binhack behavior)" << endl
         << endl
         << "  " << PROGNAME << " hack0 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK0 (by kikuchan, from hack4 v1.5, 2001/05/04): replaces every raw" << endl
         << "      old-lba reference with lba directly (no +166/+150 offset)" << endl
         << endl
         << "  " << PROGNAME << " hack1|hack <boot.bin> <lba> [old-lba]" << endl
         << "      HACK1 (by Bero): replaces every (old-lba+166) reference with (lba+166)" << endl
         << endl
         << "  " << PROGNAME << " hack2 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK2 (by Unknown): replaces every (old-lba+150) reference with" << endl
         << "      (lba+150)" << endl
         << endl
         << "  " << PROGNAME << " hack3 <boot.bin> <lba> [old-lba]" << endl
         << "      HACK3 (by Pekearai / Mr.KiMWU, 2001-02-23): HACK1 + HACK2 combined" << endl
         << endl
         << "  " << PROGNAME << " dahack <boot.bin> <lba> [old-lba]" << endl
         << "      DAHACK (by Mr.KiMWU, 2001-02-23): HACK1(lba) + HACK2(0)" << endl
         << endl
         << "  " << PROGNAME << " cdda <boot.bin>" << endl
         << "      CDDA fix (by Mr.KiMWU, 2001-02-23): fixes multi-track CDDA bootbins" << endl
         << "      where the first audio track reads as track04 instead of track01" << endl
         << endl
         << "  " << PROGNAME << " bincon <boot.bin>" << endl
         << "      bincon (by dopefish, 7/28/00): voodoo to make a raw WinCE boot binary" << endl
         << "      bootable. Run binhack-ip afterward to also set IP.BIN's OS flag: it" << endl
         << "      detects bincon'd binaries automatically, no separate flag needed" << endl
         << endl
         << "  " << PROGNAME << " unprotect <boot.bin> <id>" << endl
         << "      Removes one of 7 non-LBA copy protections. <id> is 0-6 or \"jsr\"" << endl
         << "      (alias for 6); see README.md for the full credited list" << endl
         << endl
         << "  " << PROGNAME << " check-protection <boot.bin> [id]" << endl
         << "      Read-only: reports whether <id>'s original and/or already-cracked" << endl
         << "      pattern is present, without modifying the file. Omit <id> to scan" << endl
         << "      all 7 recognized protections" << endl
         << endl
         << "  " << PROGNAME << " wince-cdda-fix-ip <ip.bin>" << endl
         << "      WinCE+CDDA fix (by pitito): fixes IP.BIN so CDDA audio doesn't break" << endl
         << "      (e.g. always replaying track 1) when converting a WinCE game from GDI" << endl
         << "      to CDI. Unlike every command above, this patches IP.BIN itself, not" << endl
         << "      the boot binary: run it after binhack-ip/binhack" << endl
         << endl
         << "  " << PROGNAME << " help" << endl
         << "      Show usage. Also accepted as --help or -h" << endl
         << endl
         << "  " << PROGNAME << " version" << endl
         << "      Show version information. Also accepted as --version or -v" << endl
         << endl
         << "Options:" << endl
         << endl
         << "  --backup" << endl
         << "      Before patching, copy each file the command modifies to <file>"
         << BACKUP_SUFFIX << "." << endl
         << "      May be placed anywhere on the command line. An existing backup is" << endl
         << "      kept, so it still holds the original after several patches" << endl;
}

unsigned int parseLba(const char* value) {
    return static_cast<unsigned int>(atoi(value));
}

// Follows the original interactive prompt sequence: binary name, then
// bootsector name, then (only if not WinCE) the msinfo/LBA value. Both
// files are patched in place. Uses the mid-level building blocks directly,
// since runBinhack() can't defer the LBA prompt until after WinCE
// detection.
int runInteractive() {
    fstream boot;
    unsigned int bootsize;
    vector<unsigned int> hackoffsets;
    unsigned int lba = 0;
    string bootname, ipname;
    char iphackbuf[BOOTSECTOR_SIZE];
    bool isWinCEBinary = false;

    printBanner();
    cout << "Running binhack in interactive mode. Both files are patched in place." << endl
         << "For other patching methods, run: \"" << PROGNAME << " help\"." << endl
         << endl;

    cout << "Enter name of binary: ";
    cin >> bootname;

    cout << "Enter name of bootsector: ";
    cin >> ipname;

    if (!ensureBackup(bootname) || !ensureBackup(ipname)) {
        return ExitCode::BackupError;
    }

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
    if (!loadIpBin(ipname, iphackbuf)) {
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

    // Pull --backup out of the argument list wherever it appears, so every
    // command below keeps its plain positional argument count.
    char* filtered[64];
    int nargs = 0;
    for (int i = 0; i < argc && nargs < 64; i++) {
        if (i > 0 && string(argv[i]) == "--backup") {
            setBackupEnabled(true);
        } else {
            filtered[nargs++] = argv[i];
        }
    }
    argv = filtered;
    argc = nargs;

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
        printBanner();
        return runBinhackBoot(argv[2], parseLba(argv[3]));
    }

    if (command == "binhack-ip") {
        if (argc != 4) {
            cout << "Usage: " << PROGNAME << " binhack-ip <boot.bin> <ip.bin>" << endl;
            return ExitCode::UsageError;
        }
        printBanner();
        return runBinhackIp(argv[2], argv[3]);
    }

    if (command == "binhack") {
        if (argc != 5) {
            cout << "Usage: " << PROGNAME << " binhack <boot.bin> <ip.bin> <lba>" << endl;
            return ExitCode::UsageError;
        }
        printBanner();
        return runBinhack(argv[2], argv[3], parseLba(argv[4]));
    }

    if (command == "hack0" || command == "hack" || command == "hack1" ||
        command == "hack2" || command == "hack3" || command == "dahack") {
        if (argc != 4 && argc != 5) {
            cout << "Usage: " << PROGNAME << " " << command << " <boot.bin> <lba> [old-lba]" << endl;
            return ExitCode::UsageError;
        }
        printBanner();
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
        printBanner();
        return runCdda(argv[2]);
    }

    if (command == "bincon") {
        if (argc != 3) {
            cout << "Usage: " << PROGNAME << " bincon <boot.bin>" << endl;
            return ExitCode::UsageError;
        }
        printBanner();
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
        printBanner();
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
        printBanner();
        return runCheckProtection(argv[2], variant);
    }

    if (command == "wince-cdda-fix-ip") {
        if (argc != 3) {
            cout << "Usage: " << PROGNAME << " wince-cdda-fix-ip <ip.bin>" << endl;
            return ExitCode::UsageError;
        }
        printBanner();
        return runWinceCddaFix(argv[2]);
    }

    cout << "Unknown command: " << command << endl << endl;
    printUsage();
    return ExitCode::UsageError;
}
