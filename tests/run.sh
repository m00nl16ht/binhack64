#!/bin/bash
#
# binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
# Copyright (C) FamilyGuy and contributors
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
echo "-- patch-boot / patch-ip / patch-all (Katana) --"

cp "$FIXTURES/1ST_READ.BIN" boot_only.bin
"$BIN" patch-boot boot_only.bin 12345 >/dev/null; rc=$?
assert_exit_code "patch-boot exits 0" 0 "$rc"
assert_files_differ "patch-boot modifies the boot binary" "$FIXTURES/1ST_READ.BIN" boot_only.bin
if [ -e IP.BIN ]; then
    not_ok "patch-boot must not read/create IP.BIN"
else
    ok "patch-boot does not touch IP.BIN"
fi

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/1ST_READ.BIN" boot_for_ip.bin
"$BIN" patch-ip boot_for_ip.bin ip_only.hak >/dev/null; rc=$?
assert_exit_code "patch-ip exits 0" 0 "$rc"
assert_files_equal "patch-ip leaves the boot binary untouched" "$FIXTURES/1ST_READ.BIN" boot_for_ip.bin
assert_file_size "patch-ip output is a full 32768-byte IP.BIN" ip_only.hak 32768
assert_byte_at_offset "patch-ip leaves the OS flag untouched (non-bincon'd)" ip_only.hak 62 00
rm -f IP.BIN

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/1ST_READ.BIN" boot_all.bin
"$BIN" patch-all boot_all.bin ip_all.hak 12345 >/dev/null; rc=$?
assert_exit_code "patch-all exits 0" 0 "$rc"
rm -f IP.BIN

# Composability: patch-boot + patch-ip run separately must equal patch-all.
assert_files_equal "patch-boot output matches patch-all's boot output" boot_only.bin boot_all.bin
assert_files_equal "patch-ip output matches patch-all's IP output" ip_only.hak ip_all.hak

# -----------------------------------------------------------------------------
echo "-- patch-boot / patch-ip (WinCE) --"

cp "$FIXTURES/0WINCEOS.BIN" wince_boot.bin
"$BIN" patch-boot wince_boot.bin 12345 >/dev/null; rc=$?
assert_exit_code "patch-boot exits 0 for a WinCE binary" 0 "$rc"
assert_files_equal "patch-boot leaves a WinCE binary untouched (no LBA hack)" \
    "$FIXTURES/0WINCEOS.BIN" wince_boot.bin

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/0WINCEOS.BIN" wince_for_ip.bin
"$BIN" patch-ip wince_for_ip.bin wince_ip.hak >/dev/null; rc=$?
assert_exit_code "patch-ip exits 0 for a WinCE binary" 0 "$rc"
assert_byte_at_offset "patch-ip leaves the OS flag untouched (non-bincon'd WinCE)" wince_ip.hak 62 00
rm -f IP.BIN

# -----------------------------------------------------------------------------
echo "-- patch-ip (bincon'd) --"

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/BINCONED.BIN" bincon_boot.bin
"$BIN" patch-ip bincon_boot.bin bincon_ip.hak >/dev/null; rc=$?
assert_exit_code "patch-ip exits 0 for a bincon'd binary" 0 "$rc"
assert_files_equal "patch-ip leaves the bincon'd boot binary untouched" \
    "$FIXTURES/BINCONED.BIN" bincon_boot.bin
assert_byte_at_offset "patch-ip sets the OS flag to '0' for a bincon'd binary" bincon_ip.hak 62 30
rm -f IP.BIN

# -----------------------------------------------------------------------------
echo "-- interactive mode --"

cp "$FIXTURES/IP.BIN" IP.BIN
cp "$FIXTURES/1ST_READ.BIN" boot_interactive.bin
printf 'boot_interactive.bin\nip_interactive.hak\n12345\n' | "$BIN" >/dev/null; rc=$?
assert_exit_code "interactive mode exits 0" 0 "$rc"
assert_files_equal "interactive mode boot output matches patch-all" boot_all.bin boot_interactive.bin
assert_files_equal "interactive mode IP output matches patch-all" ip_all.hak ip_interactive.hak
rm -f IP.BIN

# -----------------------------------------------------------------------------
echo "-- error paths --"

"$BIN" patch-boot only_one_arg >/dev/null 2>&1; rc=$?
assert_exit_code "patch-boot with wrong arg count" 1 "$rc"

"$BIN" patch-ip only_one_arg >/dev/null 2>&1; rc=$?
assert_exit_code "patch-ip with wrong arg count" 1 "$rc"

"$BIN" patch-all one two >/dev/null 2>&1; rc=$?
assert_exit_code "patch-all with wrong arg count" 1 "$rc"

"$BIN" not-a-real-command >/dev/null 2>&1; rc=$?
assert_exit_code "unknown subcommand" 1 "$rc"

"$BIN" patch-boot does_not_exist.bin 12345 >/dev/null 2>&1; rc=$?
assert_exit_code "patch-boot on a nonexistent file" 2 "$rc"

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
