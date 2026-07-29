# Changelog

All notable changes to `binhack64` are documented in this file. The format
follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); this
project continues binhack32's `X.Y.Z.W` version scheme.

## [2.0.0.0] - 2026-07-29

binhack64 is a ground-up modernization of binhack32: a native 64-bit build,
a redesigned CLI, and independent BOOT.BIN/IP.BIN patching.

### Added

- Native 64-bit build.
- A git-style command-line interface (binhack32 was interactive-only, with
  no command-line arguments at all), plus `help` / `--help` / `-h` and
  `--version` / `-v`.
- `binhack-boot`, `binhack-ip` and `binhack` subcommands, splitting the
  classic BINHACK patch into its two halves so they can be run
  independently instead of always together. `binhack-boot` never touches
  `IP.BIN`, and `binhack-ip` never modifies `BOOT.BIN`.
- `hack0`, `hack1`, `hack2`, `hack3` and `dahack` subcommands: alternate
  LBA patches that only ever touch the boot binary. All accept an optional
  `[old-lba]` (default 45000). `hack` is accepted as an alias for `hack1`.
- `cdda` subcommand: fixes multi-track CDDA boot binaries where the first
  audio track reads as track04 instead of track01.
- `bincon` subcommand: makes a raw Windows CE boot binary bootable.
- `unprotect <id>` subcommand: removes one of 7 non-LBA copy protections
  (`id` 0-6, or `jsr` as an alias for 6). See CREDITS.md for the full
  credited list.
- `check-protection <boot.bin> [id]` subcommand: reports whether a
  protection's original or cracked pattern is present, without modifying
  the file. Omit `id` to scan all 7.
- `wince-cdda-fix-ip` subcommand: fixes `IP.BIN` so CDDA audio doesn't break
  when converting a Windows CE game from GDI to CDI. Unlike every other
  patch command it targets `IP.BIN`, so run it after `binhack-ip` or
  `binhack`. Applying it twice is a no-op rather than an error, and it
  refuses to touch a file without the expected marker.
- `--backup` option: copies each file a command modifies to `<file>.bak`
  first. Accepted anywhere on the command line. An existing backup is kept
  rather than overwritten, so it still holds the original after several
  patches have been run over the same file.
- `make test`: a black-box test suite, also run in CI on every push and
  pull request.
- `make dist`: strips and (best-effort) UPX-compresses the binary for
  release, separately from a plain development build. Packing is skipped
  with a warning if UPX isn't installed, rather than failing the build.

### Differences from binhack32

- `IP.BIN` is patched in place, like `BOOT.BIN` always was. binhack32 read
  a template called `IP.BIN` from the current directory and wrote the
  result to the name you gave it; binhack64 reads nothing you didn't name.
  Copy your template first to keep it.
- A missing `IP.BIN`, or one that isn't a full 32768-byte bootsector, is an
  error rather than a corrupt result. binhack32 also read `IP.BIN` as text
  on Windows, which silently dropped CR bytes and shifted the rest.
- `cdda`, `bincon` and `wince-cdda-fix-ip` refuse to write when the target
  file doesn't match what the patch expects.
- Exit codes are small, named, non-negative values (0 for success), instead
  of ad-hoc negative numbers.
- The no-argument interactive mode keeps binhack32's original prompts and
  order, including only asking for the LBA/msinfo value when the boot
  binary isn't Windows CE.

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
