/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Command-line front-end: parses argv into a subcommand and dispatches
	to commands.hpp, or falls back to the legacy interactive prompts when
	invoked with no arguments.
*/

#ifndef __CLI__HPP__
#define __CLI__HPP__

int runCli(int argc, char* argv[]);

#endif // __CLI__HPP__
