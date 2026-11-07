# Developing Burnout 3: Takedown

How the port is built and how to work on it. Players want the
[README](../README.md) and [playing.md](playing.md).

## How the port works

The game's x86 code is lifted to C by the
[xboxrecomp](https://github.com/apexJCL/xboxrecomp) toolkit and compiled
together with a replacement Xbox kernel, GPU (NV2A) and audio (APU) runtime,
driven by [xboxrecomp-cli](https://github.com/apexJCL/xboxrecomp-cli). No
CPU emulator, no disc in a drive: one command builds a private, installable
bundle from your disc dump.

## Build from source

```sh
./burnout3 setup      # once: .venv, llvm-mingw, the toolkit (each download sha256-pinned)
./burnout3 all        # analyze (parse, disasm, funcid, abi), recomp, build
./burnout3 doctor     # what this machine has, and what is next
```

Windows: `burnout3` (the `burnout3.cmd` wrapper) or `py -3 burnout3.py`, from a
short checkout path such as `C:\g\burnout3`, with
`git config --global core.longpaths true`.

The build is `build-win/burnout3.exe`, cross-compiled with llvm-mingw
on any host, or `build/burnout3` with Apple clang on a Mac (Metal).
`src/game/main.c` holds this title's glue,
`config/seed_functions.json` the function starts the disassembler missed,
and `./burnout3 analyze` then `./burnout3 recomp` regenerate
`src/game/recomp/gen/`, the same files upstream's `regen.sh` writes (never
edit it by hand). The CLI's `docs/porting.md` has the loop;
`game.toml` has every setting, documented in the CLI's `docs/manifest.md`.
This game's environment keys, the pad script and the capture tools are in
[running.md](running.md); [architecture.md](architecture.md) says which part
owns what, and [boot.md](boot.md) what it took to get here, fix by fix.

Every command:

```
./burnout3 setup        # the tools venv (.venv), llvm-mingw, the toolkit at its pin
./burnout3 doctor       # what this host has
./burnout3 analyze      # xbe_parser, disasm, func_id, abi_analysis -> build/xr/
./burnout3 recomp       # the C, into src/game/recomp/gen/ (the same files regen.sh writes)
./burnout3 build        # build-win/burnout3.exe; `build macos` on a Mac: build/burnout3
./burnout3 bench ...    # build and run it under Proton on a Linux host (running.md)
./burnout3 package windows   # dist/: an installer folder for Windows
./burnout3 package steamos   # dist/: one .tar for a SteamOS-like Linux PC (Proton through umu-run)
./burnout3 package macos     # dist/: a .dmg with Burnout3Takedown.app (on a Mac)
./burnout3 wizard build --dev   # the player wizard from this tree (docs: the CLI's docs/wizard.md)
```

The disc is read from `Burnout 3 Takedown/`; nothing is written into it. A `../xboxrecomp` checkout beside this one is used as it is
(`XBOXRECOMP_DIR` names another), like a `../xboxrecomp-cli` checkout for the
CLI (`XBOXRECOMP_CLI_DIR`).

`./burnout3 package` (with no target: `windows`) builds a stock exe in its own
`build-pkg-win/` and wraps it with a launcher, your game files and a README
into a private bundle in `dist/`. The bundle holds your copy of the game:
it is for your own machines, never to share. The launcher renders through
Direct3D 11 (`RECOMP_PB_BACKEND=d3d11`); settings and saves live outside the
program, in `%LOCALAPPDATA%\Burnout3Takedown` on Windows and
`~/Games/Burnout3Takedown` on Linux, and survive updates. The bundle's
`README.txt` has the install steps.

The frame pacing default depends on the build: the Windows exe,
and so the SteamOS bundle under Proton, keeps the stock `spin`, and the macOS
app sleeps (`sleep`). `config.default.toml` leaves the key out so that the
per-build default applies. As a result `./burnout3 package` no longer adds its
pacing paragraph to `README.txt` nor prints the note: the CLI's
`pacing_sleeps` reads only the defaults file. That is right for the Windows
and SteamOS bundles, which spin; the macOS app's README loses its sleep
paragraph until the CLI learns the per-target default.

## The toolchain and its pins

| Component | Pinned at | Where |
|---|---|---|
| xboxrecomp-cli (`xbr`) | tag `v0.21.0`, commit `c0ae467` | `game.toml` `[cli]` |
| xboxrecomp toolkit (fork) | tag `blinx2-v0.14.0`, commit `7a97fcd`, branch `blinx2/portability` | `game.toml` `[toolkit]` |
| llvm-mingw | `20260922` | `game.toml` `[toolchain]` |

`./burnout3 doctor` reports what is in use and whether it is the pin.

## Upstream's route: Windows and Visual Studio

Windows with Visual Studio 2022 and upstream's toolkit,
unchanged in this fork. You need:

- Windows 10 or 11, x64, with about 12 GB free
- your own Burnout 3: Takedown disc image (`.iso`)
- an Xbox or XInput controller (two for split-screen); there is no keyboard mapping

### Quick start

1. Download this repository (green **Code** button, **Download ZIP**) and unzip it
   somewhere with space, for example `C:\recomp\burnout3`.
2. Double-click **`Setup.cmd`**.

   It checks for Python 3.10+, the capstone package, Git, CMake and the Visual
   Studio 2022 C++ build tools, and asks before installing any that are missing
   (with winget), saying what each is and how big. It fetches the xboxrecomp
   toolkit beside the repo folder, then asks for your `.iso` (it looks next to the
   repo and in Downloads first). After that it unpacks the disc, regenerates the C
   and builds.

   Each step is skipped once done, so after a failure just run it again. What went
   wrong goes in `setup.log`, and the window says what to do next.
3. Double-click **`Play Burnout 3.cmd`**, which the setup leaves in the repo folder.

The first build takes 20–40 minutes. Nothing from the game is downloaded or
bundled: the only game data used is your disc image.

### Step by step

The same steps as `Setup.cmd`, by hand. Commands are for PowerShell, except where
the step says Git Bash.

1. **Install the prerequisites.**

   | Tool | Version | Get it |
   |---|---|---|
   | Python | 3.10 or newer, with the `py` launcher | `winget install -e --id Python.Python.3.12` |
   | capstone | 5.x | `py -3 -m pip install --user capstone` |
   | Git for Windows | any recent, which includes Git Bash | `winget install -e --id Git.Git` |
   | CMake | 3.20 or newer | `winget install -e --id Kitware.CMake` |
   | Visual Studio 2022 | Build Tools with *Desktop development with C++* | `winget install -e --id Microsoft.VisualStudio.2022.BuildTools` |
   | ffmpeg | optional, for `--record` | `winget install -e --id Gyan.FFmpeg` |

   Open a **new** terminal afterwards so PATH picks them up. If `python` opens the
   Microsoft Store, that's the Store alias; use `py -3` as below, or turn the alias
   off under *Settings → Apps → Advanced app settings → App execution aliases*.

   ```
   > py -3 --version
   Python 3.12.10
   > cmake --version
   cmake version 3.31.6
   ```

2. **Put the repo and the toolkit side by side.** The build looks for the toolkit at
   `..\xboxrecomp`.

   ```
   git clone https://github.com/sp00nznet/burnout3
   git clone https://github.com/sp00nznet/xboxrecomp
   ```

   The toolkit's `main` has everything Burnout 3 needs (xboxrecomp #138-#156).

3. **Unpack your disc image** into `burnout3\Burnout 3 Takedown\`, using the
   toolkit's XDVDFS reader:

   ```
   cd xboxrecomp
   py -3 -m tools.xiso unpack "D:\isos\Burnout 3 - Takedown (USA).iso" -o "..\burnout3\Burnout 3 Takedown"
   cd ..\burnout3
   ```

   You should end up with `Burnout 3 Takedown\default.xbe` next to `Data\`,
   `Tracks\`, `ovid\` and the rest (about 2.4 GB).

4. **Regenerate the C** (Git Bash, from the `burnout3` folder):

   ```
   $ ./regen.sh --disasm
   ==> xbe_parser
   ==> disasm
   ==> func_id
   ==> abi_analysis
   ==> recomp
   ==> done. Now: cmake --build build-xr --config Release
   ```

   This writes `src/game/recomp/gen/`, about 29 files, all gitignored.

5. **Build** (PowerShell or Git Bash):

   ```
   cmake -S . -B build-xr -G "Visual Studio 17 2022" -A x64
   cmake --build build-xr --config Release --parallel 2
   ```

   It ends with `burnout3.vcxproj -> ...\burnout3\bin\burnout3.exe`. `--parallel 2`
   together with the default `/MP` (half your cores) keeps the machine usable while
   it compiles. Each generated file is large at `/O2`.

6. **Run** it from the repo folder, so it finds `Burnout 3 Takedown\` and `saves\`:

   ```
   .\bin\burnout3.exe
   ```

   A window opens titled `Burnout 3: Takedown | FPS: n | draws: n`, and you see the
   EA and Criterion intros, then "Press START".

**Usual trip-ups:** `regen.sh` needs Git Bash, not WSL's `bash`. A tool installed a
moment ago isn't found until you open a new window. The build needs the
64-bit generator (`-A x64`).

### Usage

```
bin\burnout3.exe [--headless] [--record out.mp4] [--seconds N] [path\to\default.xbe]
```

| Invocation | What it does |
|---|---|
| `burnout3.exe` | Plays in a window. Pad 1 and pad 2 are XInput pads 1 and 2. |
| `burnout3.exe --headless` | Runs with no window, for scripts and remote sessions. |
| `burnout3.exe --headless --record out.mp4 --seconds 600` | Runs for 10 minutes with no window, then encodes every presented frame to `out.mp4` (needs `ffmpeg` on PATH). Ctrl+C also stops and encodes. |

Scripted input, for recording or testing without a pad in hand:

```
set RECOMP_PAD_SCRIPT=20000:start:1200,30000:a:400,90000:rt:60000,90000:p2-rt:60000
bin\burnout3.exe --headless --record race.mp4 --seconds 900
```

Each entry is `<ms>:<button>[+<button>][:<hold ms>]`, timed from when the game first
reads the pad, and a `p2-` prefix sends it to the second pad. `RECOMP_PAD_LIVE=pad.txt` instead reads steps appended to a file while the
game runs. [running.md](running.md) lists the buttons, the environment
variables and the capture tools.

The emulated hard disk lives in `saves\`. Delete it to see the first-boot flow
(profile creation) again.

### How upstream's build works

`regen.sh` runs the toolkit's pipeline on `default.xbe`. The stages are XBE parser,
disassembler, library-function identification, ABI analysis and the recompiler.
CMake then builds the generated C against the xboxrecomp runtime. That runtime
supplies the Xbox kernel, a software NV2A GPU, the MCPX audio processor, a USB
OHCI host with emulated pads, and the XMV movie player. See
[architecture.md](architecture.md) for which part owns what.

The C is regenerated, not edited. If the output is wrong, the fix goes in the
toolkit's recompiler or in this repo's per-title data
(`config/seed_functions.json`, `src/game/main.c`), never by hand in `gen/`.

The exe is a Windows GUI-subsystem program, so it opens no console window
beside the game. Started from a terminal, `cmd` no longer waits for it and
its output goes nowhere: set `RECOMP_STDIO_LOG=<file>` (fork toolkit) to keep
the log, or configure with `-DBO3_CONSOLE=ON` to build the console program,
which prints to the terminal as before (needed for Ctrl+C to stop a
`--record` run).

## Contributing

Changes are tracked with [OpenSpec](https://github.com/Fission-AI/OpenSpec)
under `openspec/changes/<name>/` (proposal, design, tasks, spec deltas).
Opt-in features default off and never change stock behaviour; generated code
is never hand-edited; environment variables go through the table and
[running.md](running.md). Read [CONTRIBUTING.md](../CONTRIBUTING.md) first,
especially *Where your code comes from*.
Toolkit changes go to
[apexJCL/xboxrecomp](https://github.com/apexJCL/xboxrecomp), branch
`blinx2/portability`; CLI changes to
[apexJCL/xboxrecomp-cli](https://github.com/apexJCL/xboxrecomp-cli). The
checks (`uv run pytest scripts`, `uv run ruff check .`, `uv run ruff format
--check .`) run on every push.

## Documentation

| Document | For |
|---|---|
| [running.md](running.md) | environment keys, the pad script, capture tools |
| [architecture.md](architecture.md) | which part of the runtime owns what |
| [boot.md](boot.md) | the boot, fix by fix |
| [ROADMAP.md](../ROADMAP.md) | the known gaps |
| [playing.md](playing.md) | building by hand, installing, options, troubleshooting |
| [CHANGELOG.md](../CHANGELOG.md) | what changed for players in each release |
