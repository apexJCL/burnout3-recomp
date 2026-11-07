## Context

BLiNX 2's change (cat openspec `guest-cpus-every-core`, cat 6ecdcfc2) has
the measurements. There the guest CPU lock already gave the game the
console's one CPU, so the pin added only its dependence on the Windows
scheduler. Burnout 3's pin had no reason of its own: it was set when the
toolkit stopped defaulting to one core (b3 005bad5, 2026-10-10) so that
the game kept running as it had.

## Decisions

- **D0. The user's decision (2026-11-05):** "Burnout 3: yes, update the
  default". Burnout 3 takes BLiNX 2's two defaults.
- **D1. Drop the game default, not the toolkit code.** `RECOMP_GUEST_CPUS`
  keeps `one` and `<n>` for the A/B. `DISPATCH_GATE` and `FLIP_PACING`
  stay game defaults: they serialise interrupts and flips, not threads.
- **D2. No guest lock.** BLiNX 2 needed the lock for its light-pool race;
  Burnout 3 has no known race of that kind, the matrix measured the
  lock neutral for it and kept it off, and upstream's Burnout 3 and the
  macOS build (no thread affinity) already run on every core without it.
- **D3. Pacing as a code default, per build.** As cat's D5: the call sits
  in `host_enhance_init` under `#ifdef _WIN32`, before `xbox_enhance_init`,
  inside `RECOMP_ENV_HAVE_ENHANCE_KEYS` (so upstream's toolkit, which has
  no layer, never sees it). A `RECOMP_ENV_GAME_DEFAULTS` row would not do:
  it fills the environment tier, which beats the player's
  `config/config.toml`. `config.default.toml` is one file for every target,
  so it leaves the key out.
- **D4. The race gate is unchanged.** It pins `RECOMP_CONFIG=none` and
  `RECOMP_PRESENT_PACING=spin` and checks the settings line, so its runs
  read the same on every build.

## Backends

D3D11 and Metal take the same defaults through the same code; the build
(Windows or not) decides pacing, not the backend.

## Risks

- A guest race that the pin hid on Windows and Linux could show up. The
  macOS build has run unpinned since it exists; the boot runs check the
  Windows exe natively and under Proton.
- The macOS bundle's README loses its frame-pacing paragraph. The CLI's
  `pacing_sleeps` (xboxrecomp-cli `package/__init__.py`) adds the pacing
  README text and note only when `config.default.toml` sets
  `pacing = "sleep"`, and the key is now out of it, though macOS still
  sleeps. The Windows and SteamOS bundles spin, so they need no note. The
  per-target note is the tracker item (the same trade-off as BLiNX 2's); it is
  recorded in `docs/developers.md`.
- The public pin (`blinx2-v0.14.0`) has no `enhance_cfg_set_code_default`:
  a Windows build at that pin does not link until the pin moves at the next
  push round. Local builds use the toolkit checkout (`XBOXRECOMP_DIR`).
