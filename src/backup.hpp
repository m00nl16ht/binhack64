/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Optional --backup support. Every patch command writes in place, so this
	keeps a copy of the untouched file next to it.
*/

#ifndef __BACKUP__HPP__
#define __BACKUP__HPP__

#include <string>

#define BACKUP_SUFFIX ".bak"

// Enabled once by the CLI when --backup is present on the command line.
void setBackupEnabled(bool enabled);
bool backupEnabled();

// Copies path to path + BACKUP_SUFFIX before it gets patched. A no-op when
// --backup wasn't given.
//
// An existing backup is kept rather than overwritten, so it still holds the
// original after a chain of patches (dahack, then cdda, then binhack) has
// been run over the same file.
//
// Returns false only if the backup was wanted but couldn't be written -
// callers must then refuse to patch, since patching is exactly what the
// backup was meant to guard against.
bool ensureBackup(const std::string& path);

#endif // __BACKUP__HPP__
