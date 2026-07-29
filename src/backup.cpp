/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Backup support implementation.
*/

#include "backup.hpp"
#include <iostream>
#include <fstream>

using namespace std;

namespace {

bool enabled = false;

bool fileExists(const string& path) {
    ifstream f(path.c_str(), ios::in | ios::binary);
    return f.is_open();
}

} // namespace

void setBackupEnabled(bool value) {
    enabled = value;
}

bool backupEnabled() {
    return enabled;
}

bool ensureBackup(const string& path) {
    if (!enabled) {
        return true;
    }

    const string bakname = path + BACKUP_SUFFIX;

    if (fileExists(bakname)) {
        cout << "Backup " << bakname << " already exists, keeping it." << endl;
        return true;
    }

    ifstream src(path.c_str(), ios::in | ios::binary);
    if (!src.is_open()) {
        // Nothing to back up: let the command itself report the missing file.
        return true;
    }

    ofstream dst(bakname.c_str(), ios::out | ios::binary | ios::trunc);
    if (!dst.is_open()) {
        cout << "Error creating backup " << bakname << "." << endl;
        return false;
    }

    dst << src.rdbuf();

    if (!dst.good()) {
        cout << "Error writing backup " << bakname << "." << endl;
        return false;
    }

    cout << "Created backup " << bakname << "." << endl;
    return true;
}
