/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) 2010-2026 FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	Entry point - see cli.hpp/cli.cpp for argument parsing and dispatch.
*/

#include "cli.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    return runCli(argc, argv);
}
