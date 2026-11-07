## 1. Change

- [x] 1.1 The user's decision: BLiNX 2's defaults for Burnout 3 (2026-11-05)
- [x] 1.2 `src/env/recomp_env_game.h`: drop `D(GUEST_CPUS, "one")`; rewrite the comments
- [x] 1.3 `src/game/main.c`: `enhance_cfg_set_code_default("present.pacing", "spin")` on `_WIN32` before `xbox_enhance_init`
- [x] 1.4 `packaging/config.default.toml` leaves `pacing` out and names both per-build values
- [x] 1.5 docs: `docs/running.md`, `docs/playing.md`

## 2. Verification

- [x] 2.1 `uv run pytest scripts`; `uv run ruff check . && uv run ruff format --check .`
- [x] 2.2 Mac build (Metal) on toolkit d1fd3480; Windows exe built on the PC
- [x] 2.3 120 s boot runs, new default against old (`RECOMP_GUEST_CPUS=one`, and `RECOMP_PRESENT_PACING=sleep` on Windows): Metal, the Deck under Proton, native Windows; no `[FAULT]`, flips/s, the settings line (`spin` on Windows and the Deck, `sleep` on the Mac)
- [x] 2.4 Native Windows audio: no starvation, and no more clicks than the old default (the XMV movies and the race audio)

Results (exe 0ffb75033e0d built on the PC from b3 2848a62 + toolkit d1fd3480; the Mac build from the same tree): every run had no `[FAULT]` and 0 present mismatches. Native Windows, 3 alternating pairs: 60.0 flips/s in both arms; settings line `spin` new, `sleep` old; `audio-check` PASS with no stale-lap replays in 5 of 6 runs; new2 had one 32 ms host starve at 56.6 s that new3 did not repeat; click counts equal (77.0-77.1k new, 77.2k old, the detector's floor on this music); worst frame 67-94 ms new, 49-57 ms old (load gaps); process CPU 223-236% at spin, 128-159% at sleep. Deck, Proton, one pair: about 60 flips/s both (7196 and 7197 flips in 120 s), `spin` new, `sleep` old. Metal, one pair: 59.9 flips/s both, `sleep` in both (macOS sets no affinity, so the pin never acted there).

## 3. Review

- [ ] 3.1 Review before the merge
- [ ] 3.2 Move the `[toolkit]` pin past d1fd3480 at the next push round

Review follow-up (review of c1fdba0). `scripts/race_gate.py` pad route (505 s, stock env, so `spin`; `--check` on the bench run), the same exe, every core against `RECOMP_GUEST_CPUS=one`. Every run passed, with no `[FAULT]` and 0 present mismatches:

| host | arm | race flips/s | race max flip gap | host audio starves |
|---|---|---|---|---|
| Windows | every core | 57.8 | 78 ms | 0 (`audio-check` PASS) |
| Windows | one core | 56.6 | 109 ms | 1, 70.9 ms (PASS) |
| Deck | every core | 56.9 | 105 ms | 45, 524 ms |
| Deck | one core | 55.9 | 140 ms | 334, 3956 ms |

The boot runs' worst frame (67-94 ms every core, 49-57 ms one core at sleep) falls in the first 600-flip window in every run, within the first second after the window opens; the one exception is new2's 69 ms at 56 s, its single starve. Two more Windows boot runs at `RECOMP_GUEST_CPUS=one` and `spin` gave 50.3 and 52.5 ms there, so the rise follows every core, not spin. New3 also had a 44.8 ms frame at about 25 s, and the one-core runs had none past 24 ms. In the race, every core is better on both hosts.
