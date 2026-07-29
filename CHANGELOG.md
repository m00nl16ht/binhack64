# Changelog

All notable changes to `binhack64` are documented in this file. The format
follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); this
project continues binhack32's `X.Y.Z.W` version scheme.

## [2.0.0.0] - 2026-07-29

binhack64 is a ground-up modernization of binhack32: a native 64-bit build,
a redesigned CLI, and independent BOOT.BIN/IP.BIN patching.

### Added

- `binhack-boot`, `binhack-ip` and `binhack` subcommands, giving binhack64 a
  proper git-style CLI (binhack32 itself was interactive-only, with no
  command-line arguments at all). `binhack-ip` never touches `BOOT.BIN`
  (opened read-only, no `CD001` search needed at all), and `binhack-boot`
  never touches `IP.BIN` — the two halves of the classic BINHACK patch can
  now be run independently instead of always together.
- `help` / `--help` / `-h` and `--version` / `-v`.
- `make dist` target: strips and (best-effort) UPX-compresses the binary
  for release, kept separate from a plain development build.
- `make test`: a black-box test suite (`tests/run.sh`) run against fixture
  boot binaries (Katana, WinCE, bincon'd) checking exit codes, that
  `binhack-boot`/`binhack-ip` never touch the file the other owns, that they
  compose to the same result as `binhack`, and that interactive mode
  matches. Runs in CI via GitHub Actions on every push/PR.
- `hack0`, `hack`, `hack2`, `hack3`, `dahack` and `cdda` subcommands:
  scene-standard boot-binary-only patches, distinct from and independent
  of the classic BINHACK patch (none of them touch `IP.BIN`).
  `hack`/`hack2`/`hack3`/`dahack` are numeric LBA reference patches ported
  from `binhacks.py` (HACK by Bero, HACK2 by Unknown, HACK3 by Pekearai,
  DAHACK by Mr. KiMWU); `hack0` is the equivalent raw-LBA patch from
  kikuchan's hack4, which also bundles hack1/hack2/hack3 under those
  names. Dates/authorship were cross-checked against the original
  compiled tools where available: `hack4.exe`'s binary embeds the CVS tag
  `hack4.c,v 1.5 2001/05/04 19:40:44 kikuchan Exp $`, confirming both the
  version and date already credited; `hack3.exe`/`dahack.exe`/`cdda.exe`
  are all dated 2001-02-23, and the original toolkit's readme explicitly
  states DAHACK and CDDA use "the method which post by Mr.KiMWU,"
  confirming that attribution independently of `binhacks.py`'s docstring.
  `cdda` fixes multi-track CDDA bootbins where the first audio track reads
  as track04 instead of track01, by writing a small routine near the
  disc's `CD001` signature — the offset math was verified empirically
  against a real CDDA bootbin fixture, since the source it's ported from
  computes it through a byte-order round-trip that isn't obvious from
  reading the code. All six accept `[old-lba]` (default 45000) where
  applicable.
- `bincon` and `unprotect` subcommands, plus a read-only `check-protection`
  companion for `unprotect`. `bincon` (dopefish, 7/28/00) makes a raw
  WinCE boot binary bootable; its transform reads unusually
  (`original[chunk:N] ++ original[N-chunk:N]` — the first 2048-byte chunk
  is dropped and the last one ends up duplicated, not a simple "rotate
  first chunk to the end") and was ported literally rather than
  "corrected," then verified two ways: with `od` against a purpose-built
  fixture before any test assertions were written, and independently
  against dopefish's original C source (`bincon.c`) — its `fread`/`fwrite`
  calls confirm the exact same drop-first-duplicate-last behavior, so this
  isn't just a quirk of `binhacks.py`'s Python port. Note: `bincon.c` also
  gained a basic already-converted check from Shoometsu in 2008 (comparing
  the input's last two 2KB chunks); that specific check isn't ported here
  — binhack64 uses its own pre-existing bincon detection (shared with
  `binhack-ip`) instead, so only dopefish's core transform is attributed to
  this command. `unprotect <id>` cracks
  one of 7 non-LBA copy protections (`id` 0-6 or `jsr`, an alias for 6;
  see README.md for the full credited list); `check-protection <boot.bin>
  [id]` reports whether a given protection's original/cracked pattern is
  present without modifying the file (omit `id` to scan all 7).

  Internally, `hack.cpp`'s single-purpose 4-byte search/replace was
  generalized into a byte-pattern primitive (`searchReplaceBytes`, plus a
  read-only `containsBytes`) so `unprotect`'s 4/6/8-byte patterns and the
  existing 4-byte LBA patches share one implementation. WinCE detection
  also gained a `detectWinCE` variant that falls back to a header-byte
  check when no `CD001` signature exists yet (raw pre-mastered WinCE
  binaries, which `bincon` targets, legitimately may not have one).
- `wince-cdda-fix` subcommand: a 2-byte `IP.BIN` fix (pitito,
  dreamcast-talk.com) for WinCE games whose CDDA audio breaks when
  converted from GDI to CDI (typically always replaying track 1). Unlike
  every other patch command, this one operates on `IP.BIN` itself rather
  than the boot binary — the affected byte sits inside the exploit
  payload the classic BINHACK patch writes, confirmed by inspecting a
  real `binhack-ip`-produced `IP.BIN` rather than assumed from the forum
  post alone. Idempotent (a no-op, not an error, if already applied) and
  refuses to touch a file that doesn't have the expected marker, rather
  than blindly overwriting two bytes the way manual hex-editing would.
- `bincon`'s precondition failures (already Katana, already bincon'd,
  bincon'd-but-not-WinCE) exit non-zero, unlike the Python original it's
  ported from, which prints a message and returns normally — nothing got
  patched, so exiting 0 would be misleading, and a non-zero exit is more
  useful for scripting. It also refuses to run on a file too small for
  its transform to be meaningful (`bootsize <= 2048`), where the Python
  original falls into a silent no-op via negative-index clamping.
- `cdda` validates its computed patch offset stays within the file before
  writing, refusing non-CDDA bootbins instead of writing out of bounds —
  the source it's ported from admitted it never added this check.
- Exit codes are small, named, non-negative values (0 for success),
  unlike binhack32's ad-hoc negative numbers.
- The no-argument interactive mode matches binhack32's original prompts
  and order exactly, including only asking for the LBA/msinfo value when
  the boot binary isn't Windows CE.
- Native 64-bit build. The Makefile lives at the repository root (rather
  than a `source/` subdirectory) and creates `obj/`/`bin/` automatically
  instead of relying on committed placeholder files to keep empty
  directories in git; `clean`/`clean-all` are idempotent; `make dist`
  strips and (best-effort) UPX-compresses the binary separately from a
  plain development build, and gracefully skips packing if UPX isn't
  installed rather than failing outright, as binhack32's Makefile did.
  binhack32's platform-specific `linux`/`osx` targets are gone since a
  single `all` target already builds identically on every platform.
- `version.rc` and the CLI banner identify the binary as binhack64
  (binhack32's said `BINHACK32`).

## binhack32 releases

binhack32 was FamilyGuy's 32-bit clone of the original 16-bit
`BINHACK.EXE`, refactored into binhack64 above. Kept here for historical
reference.

### [1.0.0.5]

- Makefile updated for macOS compatibility (`make osx`). Thanks to
  darcagn.

### [1.0.0.4]

- Source released under the GNU General Public License v3 (GPLv3).

### [1.0.0.3]

- Linux build support: `make linux` compiles and packs the binary with
  UPX.
- Added batch and bash test scripts.
- Source released under the WTF Public License v2.

### [1.0.0.2]

- WinCE binaries already processed by `bincon.exe` now get their IP.BIN
  OS flag cleared automatically, instead of requiring a manual step.
- Fixed a 1.0.0.1 bug where bincon'd binaries were incorrectly boot-hacked
  even though they shouldn't have been touched.

### [1.0.0.1]

- Added automatic detection of bincon'd binaries (this is what the
  1.0.0.2 fix above corrects).

### [1.0.0.0] - 2010

- Initial 32-bit clone of the original 16-bit `BINHACK.EXE`, compatible
  with non-16-bit operating systems (e.g. 64-bit Windows 7).
- Always enables the VGA flag in `IP.BIN`.
