# binhack64

[![CI](https://github.com/m00nl16ht/binhack64/actions/workflows/ci.yml/badge.svg)](https://github.com/m00nl16ht/binhack64/actions/workflows/ci.yml)

> The ultimate low-level binary patch tool for the Sega Dreamcast.

**binhack64** is a modernized, 64-bit-native continuation of `binhack32`,
itself a clone of the original 16-bit `BINHACK.EXE` for the Sega Dreamcast. It
patches selfboot discs so a `BOOT.BIN` (the file that usually ships on disc
as `1ST_READ.BIN`) can be found and run at the LBA it ends up burned at, and
produces a matching `IP.BIN` (region-free, VGA-enabled, with the correct
`BOOT.BIN` size baked in).

Despite the name, binhack64 isn't limited to the classic BINHACK patch.
Running it with no arguments drops straight into that original interactive
mode, unchanged from binhack32/`BINHACK.EXE`, which is why that's still what
plain `binhack64` does. But the tool also implements several other public,
scene-standard Dreamcast patching methods as subcommands: `HACK0`, `HACK1`,
`HACK2`, `HACK3`, `DAHACK`, the `CDDA` fix, `BINCON`, and `UNPROTECT`. See
Usage below for the full list.

## What's new in binhack64

The original tool always patched `BOOT.BIN` and `IP.BIN` together in one
pass. binhack64 splits that into independent operations, since it's common
to want only one of the two: regenerating `IP.BIN`, for example, for a
`1ST_READ.BIN` you don't want to touch. It also adds a lot of patches, much
as `HACK4` did back in the day. The CLI is modernized with git-style
subcommands, while the legacy no-argument interactive mode is kept exactly
as it was.

## Usage

```
binhack64
    Patch both BOOT.BIN (1ST_READ.BIN) and IP.BIN (classic binhack
    behavior), in interactive mode: prompts for the filenames and the LBA

binhack64 binhack-boot <boot.bin> <lba>
    Patch BOOT.BIN (1ST_READ.BIN) only: writes the LBA hack

binhack64 binhack-ip <boot.bin> <ip.bin>
    Patch IP.BIN only: writes region/VGA/bincon flags and the BOOT.BIN
    size. <ip.bin> is patched in place

binhack64 binhack <boot.bin> <ip.bin> <lba>
    Patch both BOOT.BIN and IP.BIN in place (classic binhack behavior)

binhack64 hack0 <boot.bin> <lba> [old-lba]
    HACK0 (by kikuchan, from hack4 v1.5, 2001/05/04): replaces every raw
    old-lba reference with lba directly (no +166/+150 offset)

binhack64 hack1|hack <boot.bin> <lba> [old-lba]
    HACK1 (by Bero): replaces every (old-lba+166) reference with (lba+166)

binhack64 hack2 <boot.bin> <lba> [old-lba]
    HACK2 (by Unknown): replaces every (old-lba+150) reference with (lba+150)

binhack64 hack3 <boot.bin> <lba> [old-lba]
    HACK3 (by Pekearai / Mr.KiMWU, 2001-02-23): HACK1 + HACK2 combined

binhack64 dahack <boot.bin> <lba> [old-lba]
    DAHACK (by Mr.KiMWU, 2001-02-23): HACK1(lba) + HACK2(0)

binhack64 cdda <boot.bin>
    CDDA fix (by Mr.KiMWU, 2001-02-23): fixes multi-track CDDA bootbins
    where the first audio track reads as track04 instead of track01

binhack64 bincon <boot.bin>
    bincon (by dopefish, 7/28/00): voodoo to make a raw WinCE boot binary
    bootable. Run binhack-ip afterward to also set IP.BIN's OS flag: it
    detects bincon'd binaries automatically, no separate flag needed

binhack64 unprotect <boot.bin> <id>
    Removes one of 7 non-LBA copy protections. <id> is 0-6 or "jsr"
    (alias for 6); see the table below for the full credited list

binhack64 check-protection <boot.bin> [id]
    Read-only: reports whether <id>'s original and/or already-cracked
    pattern is present, without modifying the file. Omit <id> to scan
    all 7 recognized protections

binhack64 wince-cdda-fix-ip <ip.bin>
    WinCE+CDDA fix (by pitito): fixes IP.BIN so CDDA audio doesn't break
    (e.g. always replaying track 1) when converting a WinCE game from
    GDI to CDI. Unlike every command above, this patches IP.BIN itself,
    not the boot binary: run it after binhack-ip/binhack

binhack64 help
    Show usage. Also accepted as --help or -h

binhack64 version
    Show version information. Also accepted as --version or -v

Options:

--backup
    Before patching, copy each file the command modifies to <file>.bak.
    May be placed anywhere on the command line. An existing backup is
    kept, so it still holds the original after several patches
```

`<lba>` is the MSINFO/LBA value of the boot binary on the target disc image
(the same value the original `BINHACK.EXE` asked for as "msinfo"). It's
ignored for Windows CE binaries, which don't need the LBA hack.

Every command patches the files you name, in place, and reads nothing you
didn't name. That includes `IP.BIN`: unlike `binhack32` and the original
`BINHACK.EXE`, which silently read a template called `IP.BIN` from the
current directory and wrote the result somewhere else, binhack64 patches
whichever bootsector you pass it, just as it does for `BOOT.BIN`. A
bootsector that doesn't exist, or that isn't a full 32768 bytes, is an
error rather than a silently truncated result.

Use `--backup` to keep the originals. An existing `.bak` is never
overwritten, so it still holds the untouched file after a chain of patches:

```
binhack64 --backup dahack 1ST_READ.BIN 35500   # creates 1ST_READ.BIN.bak
binhack64 --backup cdda 1ST_READ.BIN           # keeps the existing .bak
```

`hack0`/`hack1`/`hack2`/`hack3`/`dahack`/`cdda`/`bincon`/`unprotect` are
alternate scene-standard patches; unlike `binhack`/`binhack-boot`/
`binhack-ip`, they only ever touch the boot binary, never `IP.BIN`.
(`wince-cdda-fix-ip` is the one exception: it only touches `IP.BIN`, and is
meant to run after one of those.) `[old-lba]` defaults to 45000, the value
most original discs were mastered at. `cdda` needs no LBA at all: it locates
itself from the boot binary's own `CD001` signature. Unlike the original
tools, `cdda` and `bincon` both refuse to write when the file doesn't
look like a valid target.

`unprotect <id>` cracks one of these non-LBA copy protections:

| id | credit | pattern |
|----|--------|---------|
| 0 | unknowns | `CD E4 43 6A` → `09 00 09 00` |
| 1 | Mr.talon | `10 32 0D 8B` → `08 00 0D 8B` |
| 2 | DCiso | `CD EB 22 D1` → `09 00 22 D1` |
| 3 | DCiso | `02 E0 04 6A` → `00 E0 04 A0` |
| 4 | DCiso | `13 E1 10 22` → `09 E1 10 22` |
| 5 | MILF & atreyu187 | `03 89 26 D3 24 D4 0B 43` → `09 00 09 00 09 00 09 00` |
| 6 (alias `jsr`) | unknowns | `0B D2 37 32 02 8B` → `0B D2 08 00 02 8B` |

`wince-cdda-fix-ip` addresses one specific, widely-applicable WinCE+CDDA
symptom at a fixed `IP.BIN` offset. The same forum thread it's ported from
also documents many per-game, per-region fixes to individual games' own
`.exe`/`.dll` files (Armada, Worms Pinball, Next Tetris, Sega Rally 2, and
more). Those are out of scope for binhack64, which patches `BOOT.BIN` and
`IP.BIN` rather than acting as a per-title patch database, but they're real
and useful if you need them: see the thread at
[Dreamcast-Talk](https://www.dreamcast-talk.com/forum/viewtopic.php?t=17407).

## Example: a typical multi-track (DA) game workflow

Documented in the original GDK toolkit's own readme, for a CDDA game where
session 2 starts at a shifted LBA (e.g. 35500):

```
binhack64 --backup dahack 1ST_READ.BIN 35500
binhack64 --backup cdda 1ST_READ.BIN
binhack64 --backup binhack 1ST_READ.BIN IP.BIN 35500
```

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
fixtures in `tests/fixtures/`, checking exit codes and exact patched bytes
for every subcommand. It runs entirely in a temporary scratch directory,
and CI runs the same suite on every push and pull request.

## License

binhack64 is free software, released under the
[GNU General Public License v3](LICENSE), same as binhack32 since v1.0.0.4.

See [CHANGELOG.md](CHANGELOG.md) for release history, including binhack32,
and [CREDITS.md](CREDITS.md) for full author/date attribution of every
patch method.

## Disclaimer

These patches mostly date back to 1999-2001, targeting a console that's
been out of production for over two decades. They're kept here for
preservation purposes: they haven't been tested against every game they
were ever used on, and they come with no warranty. As the original GDK
toolkit's own readme put it: *"Please use as self-responsibility."*
