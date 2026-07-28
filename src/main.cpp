/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Entry point - see cli.hpp/cli.cpp for argument parsing and dispatch.
*/

#include "cli.hpp"

int main(int argc, char* argv[]) {
    return runCli(argc, argv);
}
