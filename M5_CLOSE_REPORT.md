# M5 Close Report — Packaging, Presets, Validation & Release (PAS-1)

## Summary

M5 focused on **usability, repeatability, presets, QA, and release mechanics** for PAS-1. No new synthesis features or OSC schema changes; no RT-unsafe work in the audio thread.

## What changed

### Repo hygiene and release structure (M5.1)

- **.gitignore:** Added `Particles/bin/` so build artefacts from the Particles (ISTR) app are not tracked.
- **Docs/release/:** New folder with:
  - **BUILD_MACOS.md** — Xcode requirement and steps to build and run PAS-1 on macOS.
  - **RUN_DEMO.md** — How to run ISTR (Particles) and PAS-1 together (order: PAS first on port 9000, then Particles with OSC to localhost:9000).
  - **TROUBLESHOOTING.md** — HAL overload, audio device, buffer size (256–512 recommended), no sound (OSC / Test Trigger).
- **CHANGELOG.md** — Single M5 entry at repo root (factual list of packaging, presets, validation, demo mode, release).

### Preset system (M5.2)

- **UI:** Preset dropdown (8 named presets) and **Reset** button in the right column; both visible in M4 mode.
- **Presets:** Dry Click, Bright Spray, Soft Foam, Heavy Border, Center Focus, Wide Dark, Crisp Short, Default (M4). Each sets Voices, Tone (brightness), Decay (damping), Density Comp, and Center Bias only.
- **Behaviour:** Changing preset updates sliders and toggles and the synthesis engine immediately; **Test Trigger** confirms sound change without restart.
- **Docs:** `Docs/release/PRESETS.md` — Table of preset parameter values and a short A/B protocol (change preset → Test Trigger ×3 → confirm difference).

### Validation protocol (M5.3)

- **M5_VALIDATION_PROTOCOL.md** (repo root) with:
  - **Objective metrics:** From PAS (OSC/s, Hit Coverage, M2 raw/fused/enq/drop, Clip blocks/s, Active vs max voices) and from ISTR overlay (sent_osc, discarded_by_budget, candidate counts where available; or rate/cooldown drops in simpler builds).
  - **Three standard scenes:** Low density (500–1k particles), mid (2k–4k), high (8k, gesture OFF, plate_amp 0), with suggested ISTR parameters and expected PAS metric bands.
  - **Subjective checks:** Base sound stable; border darker than base; dense scenes structured; Tone/Decay audible via Test Trigger and at runtime.

### Demo mode (M5.4)

- **Toggle “Demo Mode”** in the UI: when ON, only essential controls and metrics are shown (Preset, Reset, Test Trigger, Output, Active Voices, Hit Coverage, OSC status, OSC messages). Hidden: M2 fusion stats, Clip blocks/s, Hits X/Y (Z discarded), and (in M4) Density Comp and Center Bias toggles.
- Layout unchanged; hidden controls keep their bounds to avoid glitches.

### Release closeout (M5.5)

- This report (**M5_CLOSE_REPORT.md**).
- **Tag:** `m5-packaging-stable` (annotated).
- **Branch:** `pas-m5-packaging` is intended to be frozen after the tag (no further code changes after the tag).

## Build and run

- See **Docs/release/BUILD_MACOS.md** for building PAS-1 on macOS.
- See **Docs/release/RUN_DEMO.md** for running ISTR + PAS-1 together.
- See **Docs/release/TROUBLESHOOTING.md** for common issues.

## Freeze and official close

- **Tag name:** `m5-packaging-stable` (annotated).
- **Branch:** Frozen. No further commits on the M5 branch; hotfixes only via a new branch and a new tag (never amend this tag).
- **M5 declared complete.** PAS-1 is in evaluation-grade state.
- **Next milestone:** M6 (future work, not started).

Reviewers should use:
- Tag `m5-packaging-stable`
- `Docs/release/RUN_DEMO.md`
- `Docs/release/PRESETS.md`
- `M5_VALIDATION_PROTOCOL.md`
