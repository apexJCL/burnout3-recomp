## Why

BLiNX 2 dropped its one-core pin and made `present.pacing` default to
`spin` in its Windows build (cat 6ecdcfc2, openspec
`guest-cpus-every-core` there). On native Windows 11 the pin, together with
spin pacing, starved the guest workers that feed a streamed audio ring, and
`sleep` let some host file reads stall for 100-675 ms where `spin` stayed
under 34 ms.

Burnout 3 set the same pin (`D(GUEST_CPUS, "one")`) for no reason of its
own: it came from BLiNX 2's default when the toolkit went title-neutral
(b3 005bad5, "as on the console"), and no Burnout 3 race needs it. **The
user's decision (2026-11-05): "Burnout 3: yes, update the default"**, the
same defaults as BLiNX 2.

## What Changes

- `src/env/recomp_env_game.h`: the game stops setting `GUEST_CPUS=one`;
  the toolkit's default, every core (upstream's), applies.
  `RECOMP_GUEST_CPUS=one` stays the A/B and the way back.
- `src/game/main.c` (`host_enhance_init`): under `#ifdef _WIN32`,
  `enhance_cfg_set_code_default("present.pacing", "spin")` before
  `xbox_enhance_init`, as cat's `src/main.c` does. The environment,
  `config/config.toml` and `config.default.toml` still win; the settings
  line and the in-game menu's stock value show the default in force.
- `packaging/config.default.toml` leaves `pacing` out (commented) and names
  both per-build values: `spin` in the Windows build (natively and under
  Proton), `sleep` on every non-Windows build (macOS).
- docs: `docs/running.md` (the options paragraph and the pacing row),
  `docs/playing.md` (the defaults that differ from the Xbox),
  `docs/developers.md` (the bundle README's pacing paragraph, below).
- Trade-off: with the key out of `config.default.toml`, `./burnout3 package`
  no longer writes the pacing paragraph into the macOS app's README.txt
  (the CLI's `pacing_sleeps` reads only that file). The per-target note is
  the tracker item, which is already filed.

Hosts: the Windows exe (D3D11, natively and under Proton) changes both
ways. The macOS build (Metal) does not change in behaviour: macOS has no
thread affinity, so the pin never acted there, and it keeps `sleep`.

Toolkit and CLI: no change here. It needs toolkit `posix-host/portability`
d1fd3480 or later (`enhance_cfg_set_code_default`), which the public pin
(`blinx2-v0.14.0`) predates: the pin moves at the next push round, as
BLiNX 2's does. Not meant for upstream (the enhancements layer is the
fork's).

## Capabilities

### New Capabilities
- `game-defaults`: the defaults this game gives toolkit keys for guest
  threads and frame pacing.

## Impact

- Burnout 3 has no guest CPU lock (`RECOMP_GUEST_LOCK` stays off, as the
  the tracker item guest matrix recommended), so on Windows and Linux its guest
  threads now run in parallel on several cores, as upstream's Burnout 3 and
  as the macOS build already did. The boot runs on every host check that.
- Windows players lose `sleep`'s CPU saving; `pacing = "sleep"` under
  `[present]` brings it back.
- No goldens (Burnout 3 has none): 120 s boot runs on Metal, the Deck
  under Proton and native Windows, against the old default on each.
