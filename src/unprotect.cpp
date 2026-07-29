/*
	binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
	Copyright (C) FamilyGuy and contributors
	Licensed under the GNU General Public License v3 (GPLv3).
	See the LICENSE file for the full license text.

	unprotect patch engine implementation.
*/

#include "unprotect.hpp"

namespace {

struct UnprotectVariant {
    const unsigned char* oldPattern;
    const unsigned char* newPattern;
    unsigned int length;
    const char* credit;
};

const unsigned char v0_old[] = { 0xCD, 0xE4, 0x43, 0x6A };
const unsigned char v0_new[] = { 0x09, 0x00, 0x09, 0x00 };
const unsigned char v1_old[] = { 0x10, 0x32, 0x0D, 0x8B };
const unsigned char v1_new[] = { 0x08, 0x00, 0x0D, 0x8B };
const unsigned char v2_old[] = { 0xCD, 0xEB, 0x22, 0xD1 };
const unsigned char v2_new[] = { 0x09, 0x00, 0x22, 0xD1 };
const unsigned char v3_old[] = { 0x02, 0xE0, 0x04, 0x6A };
const unsigned char v3_new[] = { 0x00, 0xE0, 0x04, 0xA0 };
const unsigned char v4_old[] = { 0x13, 0xE1, 0x10, 0x22 };
const unsigned char v4_new[] = { 0x09, 0xE1, 0x10, 0x22 };
const unsigned char v5_old[] = { 0x03, 0x89, 0x26, 0xD3, 0x24, 0xD4, 0x0B, 0x43 };
const unsigned char v5_new[] = { 0x09, 0x00, 0x09, 0x00, 0x09, 0x00, 0x09, 0x00 };
const unsigned char v6_old[] = { 0x0B, 0xD2, 0x37, 0x32, 0x02, 0x8B };
const unsigned char v6_new[] = { 0x0B, 0xD2, 0x08, 0x00, 0x02, 0x8B };

const UnprotectVariant variants[UNPROTECT_VARIANT_COUNT] = {
    { v0_old, v0_new, 4, "unknowns" },
    { v1_old, v1_new, 4, "Mr.talon" },
    { v2_old, v2_new, 4, "DCiso" },
    { v3_old, v3_new, 4, "DCiso" },
    { v4_old, v4_new, 4, "DCiso" },
    { v5_old, v5_new, 8, "MILF & atreyu187" },
    { v6_old, v6_new, 6, "unknowns, aka \"jsr\"" },
};

} // namespace

int unprotectVariantFromString(const string& arg) {
    if (arg == "jsr") {
        return 6;
    }
    if (arg.size() == 1 && arg[0] >= '0' && arg[0] <= '6') {
        return arg[0] - '0';
    }
    return -1;
}

bool applyUnprotect(fstream& boot, unsigned int bootsize, int variant, unsigned int& count) {
    if (variant < 0 || variant >= UNPROTECT_VARIANT_COUNT) {
        return false;
    }
    const UnprotectVariant& v = variants[variant];
    count = searchReplaceBytes(boot, bootsize, v.oldPattern, v.newPattern, v.length);
    return true;
}

bool checkProtection(istream& boot, unsigned int bootsize, int variant,
                     bool& isProtected, bool& isUnprotected) {
    if (variant < 0 || variant >= UNPROTECT_VARIANT_COUNT) {
        return false;
    }
    const UnprotectVariant& v = variants[variant];
    isProtected = containsBytes(boot, bootsize, v.oldPattern, v.length);
    isUnprotected = containsBytes(boot, bootsize, v.newPattern, v.length);
    return true;
}

string unprotectVariantCredit(int variant) {
    if (variant < 0 || variant >= UNPROTECT_VARIANT_COUNT) {
        return "";
    }
    return variants[variant].credit;
}
