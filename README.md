# binhack64

[![CI](https://github.com/m00nl16ht/binhack64/actions/workflows/ci.yml/badge.svg)](https://github.com/m00nl16ht/binhack64/actions/workflows/ci.yml)

`binhack64` is a modernized, 64-bit-native continuation of `binhack32`
(FamilyGuy and contributors, since 2010), itself a clone of the original
16-bit `BINHACK.EXE` for the Sega Dreamcast. It patches selfboot discs so a
homebrew `BOOT.BIN` (the file that ships on disc as `1ST_READ.BIN`) can be
found and run at the LBA it ends up burned at, and produces a matching
`IP.BIN` (region-free, VGA-enabled, with the correct `BOOT.BIN` size baked
in).

## What's new in binhack64

The original tool always patched `BOOT.BIN` and `IP.BIN` together in one
pass. binhack64 splits that into independent operations, since it's common
to want only one of the two — for example, regenerating `IP.BIN` for a
`1ST_READ.BIN` you don't want to touch. The CLI is also modernized with
git-style subcommands, while the legacy no-argument interactive mode is
kept exactly as it was.

## Usage

```
binhack64
    Interactive mode (prompts for input, patches both files)

binhack64 patch-boot <boot.bin> <lba>
    Patch BOOT.BIN (1ST_READ.BIN) only: writes the LBA hack.

binhack64 patch-ip <boot.bin> <ip.bin>
    Patch IP.BIN only: writes region/VGA/bincon flags and the BOOT.BIN
    size. The unpatched template is always read from IP.BIN in the
    current directory; <ip.bin> is the output filename.

binhack64 patch-all <boot.bin> <ip.bin> <lba>
    Patch both BOOT.BIN and IP.BIN (classic BINHACK behavior).

binhack64 hack0 <boot.bin> <lba> [old-lba]
    HACK0 (kikuchan): replaces every raw old-lba reference with lba
    directly (no +166/+150 offset).

binhack64 hack <boot.bin> <lba> [old-lba]
    HACK (Bero): replaces every (old-lba+166) reference with (lba+166).

binhack64 hack2 <boot.bin> <lba> [old-lba]
    HACK2 (Unknown): replaces every (old-lba+150) reference with (lba+150).

binhack64 hack3 <boot.bin> <lba> [old-lba]
    HACK3 (Pekearai): HACK + HACK2 combined.

binhack64 dahack <boot.bin> <lba> [old-lba]
    DAHACK (Mr. KiMWU): HACK(lba) + HACK2(0).

binhack64 cdda <boot.bin>
    CDDA fix (Mr. KiMWU): fixes multi-track CDDA bootbins where the first
    audio track reads as track04 instead of track01.

binhack64 help
    Show usage.

binhack64 --version
    Show version information.
```

`<lba>` is the MSINFO/LBA value of the boot binary on the target disc image
(the same value the original `BINHACK.EXE` asked for as "msinfo"). It's
ignored for Windows CE binaries, which don't need the LBA hack.

`hack0`/`hack`/`hack2`/`hack3`/`dahack`/`cdda` are alternate scene-standard
patches (`hack`/`hack2`/`hack3`/`dahack`/`cdda` by Bero, Unknown, Pekearai
and Mr. KiMWU; `hack0` from kikuchan's hack4) — unlike `patch-*`, they only
ever touch the boot binary, never `IP.BIN`. `[old-lba]` defaults to 45000
(how most original discs were mastered). `cdda` needs no LBA at all: it
locates itself from the boot binary's own `CD001` signature, and — unlike
the original it's ported from — refuses to write if the computed patch
location doesn't look like a real CDDA bootbin.

## Building

Requires a C++ toolchain (MinGW-w64 `g++` on Windows, or plain `g++`/`make`
on Linux/macOS). From the repository root:

```
make
```

This produces an unstripped `bin/binhack64(.exe)`, handy for debugging. For
a release binary, strip and (if [UPX](https://upx.github.io/) is installed)
compress it with:

```
make dist
```

## Testing

```
make test
```

Builds the project, then runs `tests/run.sh`: a black-box suite against the
fixtures in `tests/fixtures/` (Katana, WinCE, bincon'd, and CDDA boot
binaries, plus a synthetic fixture built to exercise `hack0`/`hack`/`hack2`)
that checks exit codes, exact patched bytes, that `patch-boot`/`patch-ip`
never touch the file the other owns and compose to the same result as
`patch-all`, that `hack3`/`dahack` compose from `hack`/`hack2` the same
way, and that interactive mode matches `patch-all`. Runs entirely in a
temporary scratch directory. CI runs the same thing on every push/PR.

## License

binhack64 is free software, released under the
[GNU General Public License v3](LICENSE), same as binhack32 since v1.0.0.4.

See [CHANGELOG.md](CHANGELOG.md) for release history, including binhack32.
