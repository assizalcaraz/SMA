# M3 – Perceptual & Timbral Refinement (Formal Closure)

## 1. Scope

- **M3** improves **perceptual clarity, timbral usefulness, and musical intelligibility** of fused collision events (M2 output), without increasing event rate or breaking RT-safety.
- **M3 does NOT change** HitAggregator behaviour (20 ms window, 4 quadrants, aggregation logic), OSC schemas, or rate limits.
- **PAS-1** is the instrument name. **M3** is a **milestone** (Perceptual & Timbral Refinement).

---

## 2. What M3 Changed

### Sound model (unified)

- **One base sound** for particle–particle collisions. **Border transform (single source of truth):** applied only when creating the FusedHitSnapshot in **HitAggregator::closeWindow**: `amplitude *= dbToGain(-3)`, `baseFreq *= 2^(-3/12)`, `brightness *= 0.92`. No duplicate adjustment in the engine.
- **Parameters KEPT:** Metalness (global), Brightness (global + from snapshot), Damping (global + from snapshot). Per-event values from FusedHitSnapshot take precedence; globals applied periodically to active voices.
- **Parameters REMOVED / BYPASSED in UI when fusion ON:** SubOsc Mix, Pitch Range, Waveform are hidden when `enableFusionAggregation` is true (they have no effect on fused path). With fusion off, they are shown for the raw-hit path.

### Voice strategy

- **maxVoices** range extended from 4–12 to **4–24** (configurable in UI). Justification: higher polyphony improves clarity at high density without raising event rate.
- **Voice stealing:** Prefer stealing voices with **residual amplitude ≤ STEAL_AMPLITUDE_THRESHOLD** (0.15). If all active voices are above threshold, steal the one with minimum residual amplitude (then oldest). Prevents stealing still-audible voices when possible.
- **CPU:** To be validated with maxVoices 16–24 in typical scenarios; no RT-unsafe operations added.

### Perceptual mapping (documented)

| Input | Audible effect |
|-------|-----------------|
| Energy (impactIntensity) | Amplitude (primary): power sum in aggregation. |
| Energy / density (count) | Brightness, damping, waveform in snapshot (secondary). |
| Border vs p2p | −3 dB, −3 semitones, slightly reduced brightness (M3). |

### UI

- **Before:** Voices (4–12), Metalness, Brightness, Damping, Waveform, SubOsc Mix, Pitch Range, Clipper, Test Trigger, and metric labels.
- **After (fusion ON):** Voices (4–24), Metalness, Brightness, Damping, Clipper, Test Trigger; metric labels including **Clip: X blocks/s** (interpretable: number of audio blocks per second in which at least one sample was soft-clipped). Waveform, SubOsc Mix, Pitch Range hidden. Title shows **"PAS-1 (M3 Perceptual)"** and subtitle **"Perceptual Mode"**.

### Metrics added

- **Clip (interpretable):** Count of **audio blocks** in which at least one sample was soft-clipped (since last reset). UI shows **"Clip: X blocks/s"** (rate computed in MainComponent timer from delta blocks over delta time). No per-sample count (avoids unreadable spam).
- Active voices, hit coverage, M2 fusion stats unchanged.

---

## 3. Success Criteria (M3)

- **Higher perceived clarity** with the **same** event rate (no change to fusion or OSC).
- **No increase** in rate-limiter or queue drops (fusedDroppedQueue, hitsDiscarded).
- **CPU stable** with maxVoices up to 24.
- **No RT-safety regressions:** no allocations, locks, or logging in audio thread.

---

## 4. Files Touched (summary)

- **FusedHitSnapshot.h:** Added `isBorder`.
- **HitAggregator.cpp:** **Single source of truth for border:** when `predominantEdge`, apply `amplitude *= 10^(-3/20)`, `baseFreq *= 2^(-3/12)`, `brightness *= 0.92` in `closeWindow` only. Set `s.isBorder = predominantEdge`.
- **SynthesisEngine:** maxVoices 4–24; `blocksClippedCount` (one increment per block if any sample clipped), `getBlocksClippedCount()`, reset in `reset()`. `processEventQueue` uses snapshot values as-is (no duplicate border tweak).
- **VoiceManager:** `STEAL_AMPLITUDE_THRESHOLD`; `findVoiceToSteal` prefers voices below threshold.
- **MainComponent:** Voices slider 4–24; hide Waveform/SubOsc/Pitch Range when fusion ON; title "PAS-1 (M3 Perceptual)" and "Perceptual Mode"; Clip label shows "Clip: X blocks/s" (rate from delta blocks / delta time).

---

## 5. Tag (to be created manually)

**Suggested tag name:**  
`m3-perceptual-stable`

**Suggested annotation:**  
"M3 complete: perceptual/timbral refinement, voice steal threshold, 4–24 voices, UI cleanup, clipper metric."
