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

binhack64 help
    Show usage.

binhack64 --version
    Show version information.
```

`<lba>` is the MSINFO/LBA value of the boot binary on the target disc image
(the same value the original `BINHACK.EXE` asked for as "msinfo"). It's
ignored for Windows CE binaries, which don't need the LBA hack.

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

Builds the project, then runs `tests/run.sh`: a black-box suite that runs
the binary against the fixtures in `tests/fixtures/` (Katana, WinCE, and
bincon'd boot binaries) and checks exit codes, that `patch-boot`/`patch-ip`
never touch the file the other owns, that they compose to the same result
as `patch-all`, and that interactive mode matches. Runs entirely in a
temporary scratch directory. CI runs the same thing on every push/PR.

## License

binhack64 is free software, released under the
[GNU General Public License v3](LICENSE), same as binhack32 since v1.0.0.4.

See [CHANGELOG.md](CHANGELOG.md) for release history, including binhack32.
