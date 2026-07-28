# Changelog

All notable changes to `binhack64` are documented in this file. The format
follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); this
project continues binhack32's `X.Y.Z.W` version scheme.

## [2.0.0.0] - 2026-07-29

binhack64 is a ground-up modernization of binhack32: a native 64-bit build,
a redesigned CLI, and independent BOOT.BIN/IP.BIN patching.

### Added

- `patch-boot`, `patch-ip` and `patch-all` subcommands, giving binhack64 a
  proper git-style CLI (binhack32 itself was interactive-only, with no
  command-line arguments at all). `patch-ip` never touches `BOOT.BIN`
  (opened read-only, no `CD001` search needed at all), and `patch-boot`
  never touches `IP.BIN` — the two halves of the classic BINHACK patch can
  now be run independently instead of always together.
- `help` / `--help` / `-h` and `--version` / `-v`.
- `make dist` target: strips and (best-effort) UPX-compresses the binary
  for release, kept separate from a plain development build.
- `make test`: a black-box test suite (`tests/run.sh`) run against fixture
  boot binaries (Katana, WinCE, bincon'd) checking exit codes, that
  `patch-boot`/`patch-ip` never touch the file the other owns, that they
  compose to the same result as `patch-all`, and that interactive mode
  matches. Runs in CI via GitHub Actions on every push/PR.

### Changed

- Rebuilt as a native 64-bit project, continuing from binhack32.
- The no-argument interactive mode is preserved exactly as before (same
  prompts, same order), including only asking for the LBA/msinfo value
  when the boot binary isn't Windows CE.
- Exit codes are now small, named, non-negative values instead of the old
  ad-hoc negative numbers.
- Makefile moved to the repository root; `obj/`/`bin/` are now created
  automatically instead of requiring pre-existing (git-tracked) empty
  directories; `clean`/`clean-all` are now idempotent; the `linux`/`osx`
  targets were removed since their recipes were identical to the default
  `all` target.
- `version.rc` and the CLI banner updated for the binhack64 rebrand.

### Fixed

- `make` no longer fails outright when UPX isn't installed — packing is
  now best-effort and only attempted as part of `make dist`.
- Translated the one remaining French comment in the codebase to English.

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
