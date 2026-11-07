## ADDED Requirements

### Requirement: Guest threads on every core

The game SHALL NOT set a default for `RECOMP_GUEST_CPUS`, so the toolkit's
default applies and the title's threads run on every host core.
`RECOMP_GUEST_CPUS=one` SHALL still pin them to one core, as the A/B.

#### Scenario: A plain Windows run

- **WHEN** the Windows build starts, natively or under Proton, with
  `RECOMP_GUEST_CPUS` unset
- **THEN** the guest threads are not pinned, and a 120 s boot run ends
  with no `[FAULT]`

### Requirement: Frame pacing per build

`present.pacing` SHALL default to `spin` in the Windows build (natively and
under Proton) and to `sleep` in every other build. The default SHALL be the
game's code default, so `RECOMP_PRESENT_PACING`, `config/config.toml` and
`config.default.toml` win over it, and the in-game menu SHALL take it as the
stock value. `config.default.toml` SHALL NOT set the key.

#### Scenario: A plain Windows run spins

- **WHEN** the Windows build starts with no `present.pacing` key in either
  settings file and no `RECOMP_PRESENT_PACING`
- **THEN** the `[ENHANCE]` settings line reads `present.pacing=spin`

#### Scenario: A plain macOS run sleeps

- **WHEN** the macOS build starts the same way
- **THEN** the `[ENHANCE]` settings line reads `present.pacing=sleep`

#### Scenario: The player's choice wins

- **WHEN** `config/config.toml` sets `pacing = "sleep"` under `[present]` on
  the Windows build
- **THEN** the settings line reads `present.pacing=sleep`
