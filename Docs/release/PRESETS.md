# PAS-1 Presets (M5)

Presets affect only the parameters visible in M4 mode: **Voices**, **Tone** (brightness), **Decay** (damping), **Density Comp**, and **Center Bias**. Changes are immediate and audible via **Test Trigger** or when receiving OSC hits.

## Preset table

| Preset name     | Voices | Tone | Decay | Density Comp | Center Bias |
|-----------------|--------|------|-------|--------------|-------------|
| Dry Click       | 8      | 0.25 | 0.25  | On           | On          |
| Bright Spray    | 12     | 0.85 | 0.45  | On           | On          |
| Soft Foam       | 10     | 0.45 | 0.75  | On           | On          |
| Heavy Border    | 16     | 0.35 | 0.85  | On           | On          |
| Center Focus    | 8      | 0.55 | 0.55  | On           | On          |
| Wide Dark       | 14     | 0.20 | 0.65  | Off          | Off         |
| Crisp Short     | 10     | 0.70 | 0.30  | On           | Off         |
| Default (M4)    | 8      | 0.50 | 0.50  | On           | On          |

- **Tone** 0 = darker, 1 = brighter.
- **Decay** 0 = shorter sustain, 1 = longer sustain.
- **Density Comp** and **Center Bias** are M4 perceptual options (see main docs).

## A/B listening protocol

1. Select a preset from the **Preset** dropdown (e.g. Dry Click).
2. Press **Test Trigger** three times and note the character (bright/dark, short/long).
3. Change to another preset (e.g. Bright Spray).
4. Press **Test Trigger** three times again.
5. Confirm an audible difference between the two presets.

Use **Reset** to restore **Default (M4)** at any time.
