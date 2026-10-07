/*
 * recomp_env_game.h - Burnout 3's keys and defaults for the fork toolkit's
 * environment table (xboxrecomp src/platform/recomp_env.h includes this file when
 * RECOMP_ENV_HAVE_GAME_KEYS is defined; CMakeLists.txt sets it on the
 * recomp_env target when the toolkit has one). Upstream's toolkit reads raw
 * getenv and has no such target, so there main.c's _putenv_s defaults apply.
 *
 * Defaults are D(ID, "value"), used when neither the key nor its old variable
 * is set in the environment.
 *   VBLANK      D3D's present waits on the vblank event the GPU ISR (vector 3)
 *               signals; without vblank delivery the boot stops after flip 1.
 *   APU_DSP_ACK auto: the DSOUND GP doorbell is SGE[0] of NV_PAPU_GPSADDR
 *               + 0x810, wherever the contiguous allocator put it (the
 *               upstream allocator gave 0x825F8810, the fork's 0x80BBC810).
 *   USB_PADS    two pads: the split-screen modes need a second controller.
 */
#ifndef RECOMP_ENV_GAME_H
#define RECOMP_ENV_GAME_H

/* This game's own keys, added to the toolkit's table (same row format:
 * X(ID, TIER, NAME, LEGACY, HELP); rows in docs/running.md). The packaged
 * launchers set both: they start the game from its logs folder, so the dump
 * and the emulated hard disk cannot be found from the working directory. */
#define RECOMP_ENV_GAME_KEYS(X) \
    X(GAME_FILES, CONFIG, "RECOMP_GAME_FILES", NULL, "game files directory (default: Burnout 3 Takedown in the working directory)") \
    X(HDD_DIR,    CONFIG, "RECOMP_HDD_DIR",    NULL, "emulated hard disk: partition images, saves (default: saves in the working directory)")

#define RECOMP_ENV_GAME_DEFAULTS(D) \
    D(VBLANK,      "1") \
    D(APU_DSP_ACK, "auto") \
    D(USB_PADS,    "2")

#endif /* RECOMP_ENV_GAME_H */
