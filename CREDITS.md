# Credits

binhack64 exists because of work spanning roughly 1999 to today across the
Dreamcast scene. This file collects full attribution for every
patch method it implements, and for binhack32/binhack64 themselves. Where
a date or name isn't listed, it isn't documented in any source available
during development, nothing here is guessed.

## Main code base

- **ECHELON**: original `BINHACK.EXE` and `IPINS.EXE`, released
  September 19, 2000 as part of the `E-SELFBT` Dreamcast self-boot
  tutorial/toolset.
- **FamilyGuy**: `binhack32` (2010), a 32-bit clone of ECHELON's
  original `BINHACK.EXE`; also `binhacks.py` (2014), the Python library
  most of the patches below were ported from.
- **MOONLiGHT**: binhack64 (2026): native 64-bit build,
  split `BOOT.BIN`/`IP.BIN` patching, git-style CLI, and the additional
  patch methods below.

## Patch methods

| Command | Trick | Author(s) | Date |
|---|---|---|---|
| `binhack` / `binhack-boot` / `binhack-ip` | BINHACK | Echelon (original `BINHACK.EXE`/`IPINS.EXE`); FamilyGuy (32-bit clone) | 2000-09-19 (original); 2010 (clone) |
| `hack0` | HACK0 | kikuchan (from `hack4` v1.5) | 2001-05-04 |
| `hack` | HACK (HACK1) | Bero | ? |
| `hack2` | HACK2 | _Unknown_ | ? |
| `hack3` | HACK3 | Pekearai / Mr.KiMWU (from GDK) | 2001-02-23 |
| `dahack` | DAHACK | Mr.KiMWU (from GDK) | 2001-02-23 |
| `cdda` | CDDA fix | Mr.KiMWU (from GDK) | 2001-02-23 |
| `bincon` | bincon | dopefish | 2000-07-28 |
| `unprotect 0` | unprotect protection 0 | _Unknown_ | 2001 |
| `unprotect 1` | unprotect protection 1 | Mr.talon | 2001 |
| `unprotect 2`–`4` | unprotect protections 2-4 | retrieved from DCiso | ? |
| `unprotect 5` | unprotect protection 5 | MILF & atreyu187 | ? |
| `unprotect 6` / `jsr` | unprotect protection 6 | _Unknown_ | ? |
| `wince-cdda-fix-ip` | WinCE+CDDA `IP.BIN` fix | pitito | 2024-04-11 |

## Notes on dates and sources

- Dates for `hack3`/`dahack`/`cdda` are the original compiled executables'
  build timestamps, not necessarily each author's exact authorship date.
- `hack0`'s date comes from a CVS tag embedded in the original
  `hack4.exe`: `hack4.c,v 1.5 2001/05/04 19:40:44 kikuchan Exp $`.
- `bincon`'s 2008 already-converted check (Shoometsu) was **not** ported:
  binhack64 uses its own bincon detection, shared with `binhack-ip`. Only
  dopefish's original transform is implemented.
- `wince-cdda-fix-ip` is ported from pitito's post on the [Dreamcast-Talk](https://www./dreamcast-talk.com)
  thread ["Wince+CDDA"](https://www.dreamcast-talk.com/forum/viewtopic.php?t=17407).
  That thread also documents per-game, per-region fixes to individual
  games' own executables/DLLs (Armada, Worms Pinball, Next Tetris, Sega
  Rally 2); those aren't implemented here (see the README for why) but
  remain credited to pitito in the thread for anyone who needs them.

## Corroboration for the unprotect patterns

Two independent sources confirm several `unprotect` variants, neither of
them derived from `binhacks.py`:

**EiFFel's "Dreamcast BinPATCH" v1.0.6** (2001) offers a "CDE44 scheme"
and a "1032 scheme," named for the same leading bytes as `unprotect 0`
and `unprotect 1`, and names two games the CDE44 one fixes: Metropolis
Street Racer and Confidential Mission. Its checkbox labels only expose
the first 2 of 4 pattern bytes, so this corroborates the schemes without
proving the two tools patch identically. BinPATCH claims no discovery,
only that it applies the crack.

**A community Dreamcast ripping database** of per-game rip notes
contributed by many individuals from 2001 onward confirms three variants
byte-for-byte:

| Variant | Pattern | Confirmed by |
|---|---|---|
| `unprotect 0` | `CD E4 43 6A` → `09 00 09 00` | Confidential Mission (PAL), Virtua Fighter 4 Passport, Phantasy Star Online (PAL), two Sonic Adventure 2 rips |
| `unprotect 1` | `10 32 0D 8B` → `08 00 0D 8B` | Virtua Fighter 4 Passport, two Sonic Adventure 2 rips |
| `unprotect 2` | `CD EB 22 D1` → `09 00 22 D1` | Virtua Fighter 4 Passport |

Two near-misses in that database are recorded here rather than claimed as
confirmation: the Phantasy Star Online entry writes `unprotect 1`'s
replacement as `08 00 0D B8` (likely a contributor's digit
transposition, since three other entries agree with the coded value), and
Virtua Fighter 4 Passport lists `02 E0 04 A0` → `00 E0 04 A0`, which
doesn't match `unprotect 3`'s search bytes (`02 E0 04 6A`).

The same database also shows why binhack64 implements no general "5EB0
scheme": Virtua Fighter 4 Passport replaces those bytes with
`96 00 00 00` while Soul Reaver (PAL) uses `A6 00 00 00`, and BinPATCH
likewise splits its 5EB0 scheme into separate selfboot/non-selfboot
variants. The replacement is per-game, not universal.

Neither source documents who originally discovered any of these
patterns, and neither resolves what `unprotect 6`'s "jsr" alias refers
to. BinPATCH does offer a "0B23 scheme" labeled for the PAL release of
Jet Set Radio, but those are different bytes than `unprotect 6`'s
`0B D2...`, it only shows that naming a scheme after a game was a real
convention at the time.

Finally, entries for Jet Set Radio (PAL) and Jet Grind Radio (US)
independently confirm `dahack`'s correctness: each documents a
`1ST_READ.BIN` patch mathematically identical to running `dahack` on
that binary. Each also lists seven further byte changes found nowhere
else in the database, a one-off patch for that game's own executable,
out of scope here.

See [CHANGELOG.md](CHANGELOG.md) for how and when each of these landed in
binhack64, and the README's `unprotect` table for the exact byte patterns
each protection variant patches.
