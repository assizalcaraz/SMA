# M5 Validation Protocol (PAS-1 + ISTR)

Objective metrics, standard test scenes, and subjective checks for evaluating PAS-1 with the ISTR particle simulation.

---

## A) Objective metrics to record

### From PAS-1 UI (overlay / labels)

- **OSC messages/s** — Incoming OSC rate (should rise with particle activity).
- **Hit Coverage (%)** — `hits_triggered / hits_received`; aim high (e.g. > 90%) when PAS is keeping up.
- **M2 raw vs fused** — Raw hits received, fused snapshots produced, enqueued, dropped; optional: cov (enqueued/raw), qloss (dropped/produced).
- **Clip blocks/s** — Blocks per second where the clipper engaged; occasional is acceptable, sustained high rate may indicate overload.
- **Active voices vs max voices** — Current polyphony vs limit (Voices slider / preset).

### From ISTR (Particles) overlay

- **Build with budgeting (e.g. Particles/ in repo root):**
  - **sent_osc** (per_sec) — OSC messages sent to PAS.
  - **discarded_by_budget** (per_sec) — Events dropped by frame budget (expected when many candidates).
  - **candidate_border** / **candidate_p2p** — Candidate counts before selection (optional).
- **Build without budgeting (e.g. entrega-istr):**
  - **osc_msgs_sent_per_sec** — Same idea as sent_osc.
  - **osc_msgs_dropped_by_rate_limiter** — Drops from rate limiter.
  - **Discarded (cooldown)** — Hits discarded by per-particle cooldown.

Record these during each test scene and note which ISTR build you used.

---

## B) Standard test scenes (3 scenes)

Each scene specifies ISTR parameter sets and expected approximate ranges for PAS metrics. Run PAS-1 first (port 9000), then ISTR with OSC target localhost:9000.

### Scene 1: Low density

- **ISTR:** N particles ≈ 500–1000, low gesture activity (few drags or low speed). Suggested: `N Particles` 500–1000, default or reduced `max_hits/s` (e.g. 200–400), `hit_cooldown (ms)` e.g. 40–60, `particle_radius` as default.
- **Expected PAS (approx):** OSC/s low (tens to low hundreds), Hit Coverage high (e.g. > 95%), few or no M2 queue drops, Clip blocks/s near 0, active voices well below max.

### Scene 2: Mid density

- **ISTR:** N ≈ 2000–4000, moderate gesture (several drags, medium speed). Suggested: `N Particles` 2000–4000, `max_hits/s` 400–600, `hit_cooldown (ms)` 40–50.
- **Expected PAS (approx):** OSC/s medium (hundreds), Hit Coverage still high (e.g. > 85%), M2 enqueued/dropped stable, Clip blocks/s low, active voices may approach max during peaks.

### Scene 3: High density (8000, gesture OFF, plate off)

- **ISTR:** N = 8000, **gesture OFF** (no mouse drag / no Chladni), **plate_amp = 0** (no /plate traffic). Suggested: `N Particles` 8000, `max_hits/s` 500–800, `hit_cooldown (ms)` 30–40, no plate excitation.
- **Expected PAS (approx):** OSC/s high but bounded (e.g. up to target_hits_per_second if ISTR uses budgeting), Hit Coverage may drop somewhat under load (e.g. 70–95% depending on buffer/hardware), some M2 queue drops acceptable; Clip blocks/s may be non-zero; sound should remain structured (no constant hash). If ISTR overlay has **discarded_by_budget**, it will be high by design.

Document the exact ISTR values used (N, max_hits/s, hit_cooldown_ms, particle_radius, etc.) and the PAS metrics observed for reproducibility.

---

## C) Subjective listening checks

- **Base sound stable and recognizable** — Default preset (or one chosen) produces a consistent, identifiable character (e.g. click–resonant in M4).
- **Border sound darker/lower than base** — Hits on borders (surfaces) are clearly darker or lower in pitch than centre/particle–particle hits.
- **Dense scenes structured** — At high density (e.g. scene 3), the result is still articulated (events and decay), not a constant wash or hash.
- **Tone/Decay changes audible** — Changing Tone and Decay (via presets or sliders) and pressing **Test Trigger** three times yields a clear perceptual difference; the same should be noticeable when running ISTR and changing presets during playback.

---

## Summary

| Section | Content |
|--------|--------|
| A | PAS: OSC/s, Hit Coverage, M2 raw–fused–enq–drop, Clip blocks/s, Active vs max voices. ISTR: sent_osc (or osc_msgs_sent_per_sec), discarded_by_budget (if available), rate/cooldown drops, candidate counts. |
| B | Three scenes: low density (500–1k), mid (2k–4k), high (8k, gesture OFF, plate_amp 0); each with ISTR params and expected PAS bands. |
| C | Subjective: stable base, border darker than base, dense but structured, Tone/Decay audible via Test Trigger and runtime. |

Use this protocol for QA and reviewer evaluation; record metrics and notes per scene for reproducibility.
