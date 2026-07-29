# Credits

binhack64 exists because of work spanning roughly 1999 to today across the
Dreamcast homebrew scene. This file collects full attribution for every
patch method it implements, and for binhack32/binhack64 themselves. Where
a date or name isn't listed, it isn't documented in any source available
during development — nothing here is guessed.

## binhack32 / binhack64

- **Echelon** — original `BINHACK.EXE` and `IPINS.EXE`, released
  September 19, 2000 as part of the `E-SELFBT` Dreamcast self-boot
  tutorial/toolset. Confirmed by both tools' own embedded banner text
  ("IP.BIN/1ST_READ.BIN Self-Boot Hacker - Echelon 2000" and "ISO
  Self-Boot Hacker - Echelon 2000") and the release's own info file; no
  more specific individual is credited anywhere in the original release.
- **FamilyGuy** — `binhack32` (2010), a 32-bit clone of Echelon's
  original `BINHACK.EXE`; also `binhacks.py` (2014), the Python library
  most of the patches below were ported from.
- **SiZiOUS** — reorganized and cleaned up FamilyGuy's original
  `binhack32` source.
- **darcagn** — macOS Makefile support (`binhack32` v1.0.0.5).
- **FamilyGuy and contributors** — binhack64 (2026): native 64-bit build,
  split `BOOT.BIN`/`IP.BIN` patching, git-style CLI, and the additional
  patch methods below.

## Patch methods

| Command | Trick | Author(s) | Date |
|---|---|---|---|
| `binhack` / `binhack-boot` / `binhack-ip` | classic BINHACK | Echelon (original `BINHACK.EXE`/`IPINS.EXE`); FamilyGuy (32-bit clone) | 2000-09-19 (original); 2010 (clone) |
| `hack0` | HACK0 | kikuchan (`hack4` v1.5) | 2001-05-04, per an embedded CVS tag in the original `hack4.exe` |
| `hack` | HACK | Bero | not documented |
| `hack2` | HACK2 | credited only as "Unknown" | not documented |
| `hack3` | HACK3 | Pekearai | `hack3.exe` (original toolkit build) dated 2001-02-23 |
| `dahack` | DAHACK | method posted by Mr.KiMWU | `dahack.exe` (original toolkit build) dated 2001-02-23 |
| `cdda` | CDDA fix | method posted by Mr.KiMWU | `cdda.exe` (original toolkit build) dated 2001-02-23 |
| `bincon` | bincon | dopefish (core transform, ported here); originally distributed via Echelon (same group as `BINHACK.EXE` above) | 2000-07-28 |
| `bincon` (not ported — see below) | bincon already-converted check | Shoometsu | 2008-01-26 |
| `unprotect 0` | unprotect protection 0 | credited only as "unknowns" | not documented |
| `unprotect 1` | unprotect protection 1 | Mr.talon | not documented |
| `unprotect 2`–`4` | unprotect protections 2-4 | retrieved from DCiso | not documented |
| `unprotect 5` | unprotect protection 5 | MILF & atreyu187 | not documented |
| `unprotect 6` / `jsr` | unprotect protection 6 | credited only as "unknowns" | not documented |

Dates for `hack3`/`dahack`/`cdda` are the original compiled executables'
build timestamps, not necessarily each author's exact authorship date —
the closest primary-source evidence available, cited as such rather than
presented as more precise than it is.

`bincon`'s 2008 already-converted check (Shoometsu) is documented here for
completeness but was **not** ported into binhack64: `bincon` uses this
codebase's own pre-existing bincon detection (shared with `binhack-ip`)
instead. Only dopefish's original transform is implemented.

See [CHANGELOG.md](CHANGELOG.md) for how and when each of these landed in
binhack64, and the README's `unprotect` table for the exact byte patterns
each protection variant patches.
