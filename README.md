# MudRunner PSP — Prototype 0.1

A small, original PSP homebrew prototype inspired by off-road truck simulation games.
This is a starter source project, **not a compiled EBOOT.PBP**.

## Prototype 0.1 scope
- PSP GU/GUM 3D rendering
- A low-poly 6-wheel truck placeholder
- Flat test terrain
- Third-person follow camera
- Basic analog-stick steering and throttle
- 30 FPS target (not yet performance-verified)

## Build environment
This project expects the PSPDEV toolchain (PSPSDK) and `psp-config` to be installed.
It cannot be compiled by a normal Android code editor alone. The included GitHub Actions
workflow is a starting point for a cloud build, but it must be tested and may need
adjustment if the upstream PSPDEV container changes.

## Local build
From a PSPDEV shell:

```sh
make
```

Expected output: `EBOOT.PBP` (after a successful build).

## Controls
- Analog stick left/right: steer
- Analog stick up/down: throttle / reverse
- START: exit

## Notes
- This first prototype uses primitive geometry, not copyrighted SpinTires assets.
- Mud deformation, wheel-ground interaction, suspension, AWD, winch, logging, audio,
  and save data are not implemented yet.
- PSP-1000 is the minimum-memory target; test all PSP models separately in PPSSPP
  and on real hardware where possible.
