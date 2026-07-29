#!/bin/bash
#
# binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
# Copyright (C) 2010-2026 FamilyGuy and contributors
# Licensed under the GNU General Public License v3 (GPLv3).
# See the LICENSE file for the full license text.
#
# Black-box test suite: drives the built binhack64 binary against the
# fixtures in tests/fixtures/ and asserts on exit codes and file content.
# Usage: tests/run.sh [path-to-binhack64-binary]

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
FIXTURES="$SCRIPT_DIR/fixtures"

BIN="${1:-}"
if [ -z "$BIN" ]; then
    if [ -x "$REPO_ROOT/bin/binhack64.exe" ]; then
        BIN="$REPO_ROOT/bin/binhack64.exe"
    elif [ -x "$REPO_ROOT/bin/binhack64" ]; then
        BIN="$REPO_ROOT/bin/binhack64"
    else
        echo "binhack64 binary not found; build it first (make)." >&2
        exit 1
    fi
fi
BIN="$(cd "$(dirname "$BIN")" && pwd)/$(basename "$BIN")"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT

PASS=0
FAIL=0

ok() {
    PASS=$((PASS + 1))
    echo "  ok - $1"
}

not_ok() {
    FAIL=$((FAIL + 1))
    echo "  FAIL - $1"
}

assert_exit_code() {
    local desc="$1" expected="$2" actual="$3"
    if [ "$actual" -eq "$expected" ]; then
        ok "$desc (exit $actual)"
    else
        not_ok "$desc (expected exit $expected, got $actual)"
    fi
}

assert_files_equal() {
    local desc="$1" a="$2" b="$3"
    if cmp -s "$a" "$b"; then
        ok "$desc"
    else
        not_ok "$desc ($a differs from $b)"
    fi
}

assert_files_differ() {
    local desc="$1" a="$2" b="$3"
    if cmp -s "$a" "$b"; then
        not_ok "$desc ($a unexpectedly identical to $b)"
    else
        ok "$desc"
    fi
}

assert_file_size() {
    local desc="$1" file="$2" expected="$3" actual
    actual="$(wc -c < "$file" | tr -d ' ')"
    if [ "$actual" = "$expected" ]; then
        ok "$desc"
    else
        not_ok "$desc (expected $expected bytes, got $actual)"
    fi
}

assert_byte_at_offset() {
    # offset is decimal, expected is a two-digit lowercase hex byte (e.g. "30")
    local desc="$1" file="$2" offset="$3" expected="$4" actual
    actual="$(od -An -tx1 -j "$offset" -N 1 "$file" | tr -d ' \n')"
    if [ "$actual" = "$expected" ]; then
        ok "$desc"
    else
        not_ok "$desc (byte at offset $offset: expected 0x$expected, got 0x$actual)"
    fi
}

assert_hex_at_offset() {
    # offset/length are decimal, expected is a lowercase hex string (no spaces)
    local desc="$1" file="$2" offset="$3" length="$4" expected="$5" actual
    actual="$(od -An -tx1 -j "$offset" -N "$length" "$file" | tr -d ' \n')"
    if [ "$actual" = "$expected" ]; then
        ok "$desc"
    else
        not_ok "$desc (bytes at offset $offset: expected $expected, got $actual)"
    fi
}

assert_contains() {
    local desc="$1" haystack="$2" needle="$3"
    case "$haystack" in
        *"$needle"*) ok "$desc" ;;
        *) not_ok "$desc (output did not contain '$needle')" ;;
    esac
}

echo "== binhack64 test suite =="
echo "binary:  $BIN"
echo "workdir: $WORKDIR"
echo

cd "$WORKDIR"

# -----------------------------------------------------------------------------
echo "-- binhack-boot / binhack-ip / binhack (Katana) --"

cp "$FIXTURES/1ST_READ.BIN" boot_only.bin
"$BIN" binhack-boot boot_only.bin 12345 >/dev/null; rc=$?
assert_exit_code "binhack-boot exits 0" 0 "$rc"
assert_files_differ "binhack-boot modifies the boot binary" "$FIXTURES/1ST_READ.BIN" boot_only.bin
if [ -e IP.BIN ]; then
    not_ok "binhack-boot must not read/create IP.BIN"
else
    ok "binhack-boot does not touch IP.BIN"
fi

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/1ST_READ.BIN" boot_for_ip.bin
"$BIN" binhack-ip boot_for_ip.bin ip_only.hak >/dev/null; rc=$?
assert_exit_code "binhack-ip exits 0" 0 "$rc"
assert_files_equal "binhack-ip leaves the boot binary untouched" "$FIXTURES/1ST_READ.BIN" boot_for_ip.bin
assert_file_size "binhack-ip output is a full 32768-byte IP.BIN" ip_only.hak 32768
assert_byte_at_offset "binhack-ip leaves the OS flag untouched (non-bincon'd)" ip_only.hak 62 00
rm -f IP.BIN

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/1ST_READ.BIN" boot_all.bin
"$BIN" binhack boot_all.bin ip_all.hak 12345 >/dev/null; rc=$?
assert_exit_code "binhack exits 0" 0 "$rc"
rm -f IP.BIN

# Composability: binhack-boot + binhack-ip run separately must equal binhack.
assert_files_equal "binhack-boot output matches binhack's boot output" boot_only.bin boot_all.bin
assert_files_equal "binhack-ip output matches binhack's IP output" ip_only.hak ip_all.hak

# -----------------------------------------------------------------------------
echo "-- binhack-boot / binhack-ip (WinCE) --"

cp "$FIXTURES/0WINCEOS.BIN" wince_boot.bin
"$BIN" binhack-boot wince_boot.bin 12345 >/dev/null; rc=$?
assert_exit_code "binhack-boot exits 0 for a WinCE binary" 0 "$rc"
assert_files_equal "binhack-boot leaves a WinCE binary untouched (no LBA hack)" \
    "$FIXTURES/0WINCEOS.BIN" wince_boot.bin

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/0WINCEOS.BIN" wince_for_ip.bin
"$BIN" binhack-ip wince_for_ip.bin wince_ip.hak >/dev/null; rc=$?
assert_exit_code "binhack-ip exits 0 for a WinCE binary" 0 "$rc"
assert_byte_at_offset "binhack-ip leaves the OS flag untouched (non-bincon'd WinCE)" wince_ip.hak 62 00
rm -f IP.BIN

# -----------------------------------------------------------------------------
echo "-- binhack-ip (bincon'd) --"

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/BINCONED.BIN" bincon_boot.bin
"$BIN" binhack-ip bincon_boot.bin bincon_ip.hak >/dev/null; rc=$?
assert_exit_code "binhack-ip exits 0 for a bincon'd binary" 0 "$rc"
assert_files_equal "binhack-ip leaves the bincon'd boot binary untouched" \
    "$FIXTURES/BINCONED.BIN" bincon_boot.bin
assert_byte_at_offset "binhack-ip sets the OS flag to '0' for a bincon'd binary" bincon_ip.hak 62 30
rm -f IP.BIN

# -----------------------------------------------------------------------------
echo "-- wince-cdda-fix --"
# ip_all.hak (from the binhack section above) is a real binhack-ip-produced
# IP.BIN - the fix only makes sense against real hacked output, not a
# hand-built fixture, since the affected byte lives inside the exploit
# payload binhack writes.

cp ip_all.hak wince_cdda_apply.bin
assert_hex_at_offset "sanity: real binhack output has the expected marker" \
    wince_cdda_apply.bin 25024 2 25c7
"$BIN" wince-cdda-fix wince_cdda_apply.bin >/dev/null; rc=$?
assert_exit_code "wince-cdda-fix exits 0" 0 "$rc"
assert_hex_at_offset "wince-cdda-fix patches the marker to 09 00" \
    wince_cdda_apply.bin 25024 2 0900

"$BIN" wince-cdda-fix wince_cdda_apply.bin >/dev/null; rc=$?
assert_exit_code "wince-cdda-fix is idempotent (exit 0 on a second run)" 0 "$rc"
assert_hex_at_offset "wince-cdda-fix leaves an already-patched file unchanged" \
    wince_cdda_apply.bin 25024 2 0900

cp "$FIXTURES/IP.BIN" wince_cdda_unrelated.bin
cp wince_cdda_unrelated.bin wince_cdda_unrelated_ref.bin
"$BIN" wince-cdda-fix wince_cdda_unrelated.bin >/dev/null 2>&1; rc=$?
assert_exit_code "wince-cdda-fix rejects a non-binhack-patched IP.BIN" 5 "$rc"
assert_files_equal "wince-cdda-fix leaves a rejected file untouched" \
    wince_cdda_unrelated_ref.bin wince_cdda_unrelated.bin

# -----------------------------------------------------------------------------
echo "-- hack0 / hack / hack2 / hack3 / dahack --"
# HACK_TEST.BIN is a synthetic fixture: 64 zero bytes with LE(45166) at
# offset 16 (the HACK target for old-lba=45000), LE(45150) at offset 32
# (the HACK2 target), and LE(45000) at offset 48 (the raw HACK0 target).
# For lba=12345: HACK0 wants lba=12345=0x3039, HACK wants lba+166=12511=
# 0x30DF, HACK2 wants lba+150=12495=0x30CF (all verified against the real
# binary before being hardcoded here).

cp "$FIXTURES/HACK_TEST.BIN" hack0.bin
"$BIN" hack0 hack0.bin 12345 >/dev/null; rc=$?
assert_exit_code "hack0 exits 0" 0 "$rc"
assert_hex_at_offset "hack0 replaces the raw HACK0 marker with lba" hack0.bin 48 4 39300000
assert_hex_at_offset "hack0 leaves the HACK marker untouched" hack0.bin 16 4 6eb00000
assert_hex_at_offset "hack0 leaves the HACK2 marker untouched" hack0.bin 32 4 5eb00000

cp "$FIXTURES/HACK_TEST.BIN" hack.bin
"$BIN" hack hack.bin 12345 >/dev/null; rc=$?
assert_exit_code "hack exits 0" 0 "$rc"
assert_hex_at_offset "hack replaces the HACK marker with (lba+166)" hack.bin 16 4 df300000

cp "$FIXTURES/HACK_TEST.BIN" hack2.bin
"$BIN" hack2 hack2.bin 12345 >/dev/null; rc=$?
assert_exit_code "hack2 exits 0" 0 "$rc"
assert_hex_at_offset "hack2 replaces the HACK2 marker with (lba+150)" hack2.bin 32 4 cf300000

cp "$FIXTURES/HACK_TEST.BIN" hack3.bin
"$BIN" hack3 hack3.bin 12345 >/dev/null; rc=$?
assert_exit_code "hack3 exits 0" 0 "$rc"

cp "$FIXTURES/HACK_TEST.BIN" hack_then_hack2.bin
"$BIN" hack hack_then_hack2.bin 12345 >/dev/null
"$BIN" hack2 hack_then_hack2.bin 12345 >/dev/null
assert_files_equal "hack3 == hack + hack2 applied in sequence" hack3.bin hack_then_hack2.bin

cp "$FIXTURES/HACK_TEST.BIN" dahack.bin
"$BIN" dahack dahack.bin 12345 >/dev/null; rc=$?
assert_exit_code "dahack exits 0" 0 "$rc"
assert_hex_at_offset "dahack replaces the HACK marker with (lba+166)" dahack.bin 16 4 df300000
assert_hex_at_offset "dahack zeroes the HACK2 marker to plain 150" dahack.bin 32 4 96000000
assert_files_differ "dahack differs from hack3 (HACK2 target differs)" dahack.bin hack3.bin

cp "$FIXTURES/HACK_TEST.BIN" hack_oldlba.bin
"$BIN" hack hack_oldlba.bin 12345 45166 >/dev/null; rc=$?
assert_exit_code "hack with an [old-lba] override exits 0" 0 "$rc"
assert_files_equal "a non-matching [old-lba] makes no changes" "$FIXTURES/HACK_TEST.BIN" hack_oldlba.bin

# -----------------------------------------------------------------------------
echo "-- cdda --"
# Offset/values below were independently computed and verified with od
# against the real fixture before being hardcoded here (see CHANGELOG).

cp "$FIXTURES/CDDA.BIN" cdda.bin
"$BIN" cdda cdda.bin >/dev/null; rc=$?
assert_exit_code "cdda exits 0 on a real CDDA bootbin" 0 "$rc"
assert_file_size "cdda does not change the file size" cdda.bin 296703
assert_files_differ "cdda modifies the file" "$FIXTURES/CDDA.BIN" cdda.bin
assert_hex_at_offset "cdda writes the routine at the computed offset" cdda.bin 208148 4 11d0224f
assert_hex_at_offset "cdda writes (val-402) after the routine" cdda.bin 208232 4 822b048c
assert_hex_at_offset "cdda writes (val+268) after that" cdda.bin 208236 4 202e048c

cp "$FIXTURES/1ST_READ.BIN" not_cdda.bin
"$BIN" cdda not_cdda.bin >/dev/null 2>&1; rc=$?
assert_exit_code "cdda rejects a non-CDDA bootbin instead of writing garbage" 3 "$rc"
assert_files_equal "cdda leaves a rejected file untouched" "$FIXTURES/1ST_READ.BIN" not_cdda.bin

# -----------------------------------------------------------------------------
echo "-- bincon --"
# WINCE_RAW.BIN is a synthetic fixture: 6144 bytes (3x2048), signature
# D6 1A (raw, not-yet-bincon'd WinCE, no CD001 - exercises detectWinCE's
# fallback), chunk1=0xAA, chunk2=0xBB, chunk3=0xCC. bincon's transform was
# run and inspected with od before being hardcoded here (see CHANGELOG):
# chunk1 is dropped, chunk2 survives, chunk3 ends up duplicated.

cp "$FIXTURES/WINCE_RAW.BIN" wince_raw.bin
"$BIN" bincon wince_raw.bin >/dev/null; rc=$?
assert_exit_code "bincon exits 0 on a raw WinCE binary" 0 "$rc"
assert_file_size "bincon does not change the file size" wince_raw.bin 6144
assert_hex_at_offset "bincon drops chunk1, chunk2 now starts at 0" wince_raw.bin 0 4 bbbbbbbb
assert_hex_at_offset "bincon: chunk2/chunk3 boundary is preserved" wince_raw.bin 2044 8 bbbbbbbbcccccccc
assert_hex_at_offset "bincon duplicates chunk3 at the end" wince_raw.bin 6140 4 cccccccc

cp "$FIXTURES/1ST_READ.BIN" bincon_katana.bin
"$BIN" bincon bincon_katana.bin >/dev/null 2>&1; rc=$?
assert_exit_code "bincon rejects a Katana binary" 3 "$rc"
assert_files_equal "bincon leaves a rejected Katana binary untouched" \
    "$FIXTURES/1ST_READ.BIN" bincon_katana.bin

cp "$FIXTURES/BINCONED.BIN" bincon_already.bin
"$BIN" bincon bincon_already.bin >/dev/null 2>&1; rc=$?
assert_exit_code "bincon rejects an already-bincon'd binary" 3 "$rc"
assert_files_equal "bincon leaves an already-bincon'd binary untouched" \
    "$FIXTURES/BINCONED.BIN" bincon_already.bin

printf 'D61A0000000000000000000000000000000000000000000000000000000000' | \
    xxd -r -p > bincon_tiny.bin
cp bincon_tiny.bin bincon_tiny_ref.bin
"$BIN" bincon bincon_tiny.bin >/dev/null 2>&1; rc=$?
assert_exit_code "bincon rejects a file too small for the transform" 3 "$rc"
assert_files_equal "bincon leaves a too-small file untouched" bincon_tiny_ref.bin bincon_tiny.bin

# -----------------------------------------------------------------------------
echo "-- unprotect / check-protection --"
# UNPROTECT_TEST.BIN is a synthetic fixture: variant 0's 4-byte marker at
# offset 0, variant 6/jsr's 6-byte marker at offset 16, zero elsewhere.

cp "$FIXTURES/UNPROTECT_TEST.BIN" unprotect0.bin
"$BIN" unprotect unprotect0.bin 0 >/dev/null; rc=$?
assert_exit_code "unprotect 0 exits 0" 0 "$rc"
assert_hex_at_offset "unprotect 0 applies the crack" unprotect0.bin 0 4 09000900

cp "$FIXTURES/UNPROTECT_TEST.BIN" unprotect6.bin
"$BIN" unprotect unprotect6.bin 6 >/dev/null; rc=$?
assert_exit_code "unprotect 6 exits 0" 0 "$rc"
assert_hex_at_offset "unprotect 6 applies the crack" unprotect6.bin 16 6 0bd20800028b

cp "$FIXTURES/UNPROTECT_TEST.BIN" unprotect_jsr.bin
"$BIN" unprotect unprotect_jsr.bin jsr >/dev/null; rc=$?
assert_exit_code "unprotect jsr exits 0" 0 "$rc"
assert_files_equal "unprotect jsr == unprotect 6" unprotect6.bin unprotect_jsr.bin

"$BIN" unprotect unprotect0.bin 7 >/dev/null 2>&1; rc=$?
assert_exit_code "unprotect rejects an out-of-range id" 1 "$rc"

"$BIN" unprotect unprotect0.bin notanid >/dev/null 2>&1; rc=$?
assert_exit_code "unprotect rejects a non-numeric id" 1 "$rc"

cp "$FIXTURES/UNPROTECT_TEST.BIN" check.bin
out="$("$BIN" check-protection check.bin 0 2>&1)"; rc=$?
assert_exit_code "check-protection 0 exits 0 before patching" 0 "$rc"
assert_contains "check-protection 0 finds the original pattern before patching" "$out" "original pattern found"

"$BIN" unprotect check.bin 0 >/dev/null
out="$("$BIN" check-protection check.bin 0 2>&1)"; rc=$?
assert_exit_code "check-protection 0 exits 0 after patching" 0 "$rc"
assert_contains "check-protection 0 finds the cracked pattern after patching" "$out" "cracked pattern found"

out="$("$BIN" check-protection "$FIXTURES/UNPROTECT_TEST.BIN" 2>&1)"; rc=$?
assert_exit_code "check-protection with no id (scan all) exits 0" 0 "$rc"
assert_contains "check-protection scan-all reports variant 0" "$out" "0 (unknowns)"
assert_contains "check-protection scan-all reports variant 6" "$out" "6 (unknowns"

# -----------------------------------------------------------------------------
echo "-- interactive mode --"

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/1ST_READ.BIN" boot_interactive.bin
printf 'boot_interactive.bin\nip_interactive.hak\n12345\n' | "$BIN" >/dev/null; rc=$?
assert_exit_code "interactive mode exits 0" 0 "$rc"
assert_files_equal "interactive mode boot output matches binhack" boot_all.bin boot_interactive.bin
assert_files_equal "interactive mode IP output matches binhack" ip_all.hak ip_interactive.hak
rm -f IP.BIN

# -----------------------------------------------------------------------------
echo "-- error paths --"

"$BIN" binhack-boot only_one_arg >/dev/null 2>&1; rc=$?
assert_exit_code "binhack-boot with wrong arg count" 1 "$rc"

"$BIN" binhack-ip only_one_arg >/dev/null 2>&1; rc=$?
assert_exit_code "binhack-ip with wrong arg count" 1 "$rc"

"$BIN" binhack one two >/dev/null 2>&1; rc=$?
assert_exit_code "binhack with wrong arg count" 1 "$rc"

"$BIN" not-a-real-command >/dev/null 2>&1; rc=$?
assert_exit_code "unknown subcommand" 1 "$rc"

"$BIN" binhack-boot does_not_exist.bin 12345 >/dev/null 2>&1; rc=$?
assert_exit_code "binhack-boot on a nonexistent file" 2 "$rc"

"$BIN" hack only_one_arg >/dev/null 2>&1; rc=$?
assert_exit_code "hack with wrong arg count" 1 "$rc"

"$BIN" hack boot.bin 123 456 789 >/dev/null 2>&1; rc=$?
assert_exit_code "hack with too many args" 1 "$rc"

"$BIN" cdda >/dev/null 2>&1; rc=$?
assert_exit_code "cdda with missing arg" 1 "$rc"

"$BIN" wince-cdda-fix >/dev/null 2>&1; rc=$?
assert_exit_code "wince-cdda-fix with missing arg" 1 "$rc"

# -----------------------------------------------------------------------------
echo "-- help / --version --"

out="$("$BIN" help 2>&1)"; rc=$?
assert_exit_code "help exits 0" 0 "$rc"
assert_contains "help output mentions usage" "$out" "Usage:"

out="$("$BIN" --version 2>&1)"; rc=$?
assert_exit_code "--version exits 0" 0 "$rc"
assert_contains "--version output mentions binhack64" "$out" "binhack64"

# -----------------------------------------------------------------------------
echo
echo "== $PASS passed, $FAIL failed =="
[ "$FAIL" -eq 0 ]
