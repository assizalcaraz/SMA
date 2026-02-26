# Changelog

## M5 – Packaging, presets, validation & release (PAS-1)

- **Repo hygiene:** `.gitignore` updated (e.g. `Particles/bin/`); release documentation scaffold in `Docs/release/`.
- **Release docs:** `BUILD_MACOS.md` (Xcode build steps), `RUN_DEMO.md` (ISTR + PAS run order), `TROUBLESHOOTING.md` (HAL overload, audio device, buffer size, OSC).
- **Presets:** Preset dropdown and Reset in UI; 6–10 named presets affecting Voices, Tone, Decay, Density Comp, Center Bias; table and A/B protocol in `Docs/release/PRESETS.md`.
- **Validation:** `M5_VALIDATION_PROTOCOL.md` with objective metrics (PAS + ISTR), three standard test scenes (low/mid/high density), and subjective listening checks.
- **Demo mode:** Toggle to show only essential controls (preset, test trigger, output, main metrics) for evaluation/presentation.
- **Release:** Tag `m5-packaging-stable`; `M5_CLOSE_REPORT.md` summarising M5 changes.
