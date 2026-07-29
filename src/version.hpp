/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Version and copyright strings, shared by the CLI banner and the Windows
	version-info resource. Plain #defines only, so version.rc can include
	this too and the two can't drift apart.

	The version number itself comes from the Makefile (VERSION), passed in
	unquoted as BINHACK64_VERSION_RAW and stringified below.
*/

#ifndef __VERSION__HPP__
#define __VERSION__HPP__

#ifndef BINHACK64_VERSION_RAW
#error "BINHACK64_VERSION_RAW must be set by the build - see VERSION in the Makefile."
#endif

#define BINHACK64_STRINGIFY_(x) #x
#define BINHACK64_STRINGIFY(x)  BINHACK64_STRINGIFY_(x)

#define BINHACK64_NAME        "binhack64"
#define BINHACK64_VERSION     BINHACK64_STRINGIFY(BINHACK64_VERSION_RAW)
#define BINHACK64_DESCRIPTION "Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker"
#define BINHACK64_COPYRIGHT   "Copyright (C) 2010-2026 FamilyGuy and contributors"
#define BINHACK64_LICENSE     "Licensed under the GNU General Public License v3 (GPLv3)."

#endif // __VERSION__HPP__
