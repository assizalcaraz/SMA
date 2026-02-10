# Prompt: Synthesizer Enhancement Implementation Plan

## Context

I'm working on a **Modal Synthesizer** built with JUCE (C++) for a real-time audiovisual system. The synthesizer is part of a modular system that receives OSC messages from an openFrameworks particle simulation and generates metallic "coin cascade" sounds.

**Current Status:**
- The synthesizer is functional but has limited timbral variety
- Several controls were removed because they didn't work properly (brightness, damping, drive, reverb)
- Only two controls remain: Voices (4-12) and Pitch/Metalness (0-1)
- The excitation is currently limited to a differentiated noise burst

## Current Architecture

**Technology Stack:**
- JUCE 8.0.12 (C++)
- macOS target
- Real-time audio processing (RT-safe)

**Core Components:**

1. **ModalVoice** (`ModalVoice.h/cpp`)
   - Implements modal synthesis with 4 resonant modes per voice
   - Each mode uses a bandpass resonant filter
   - Inharmonic factors: [1.0, 2.76, 5.40, 8.93] for metallic timbre
   - Current excitation: Differentiated noise burst (white noise with high-pass)
   - Formant filter for additional timbral character
   - Frequency variation (±2%) for richer timbre

2. **VoiceManager** (`VoiceManager.h/cpp`)
   - Manages polyphony (4-12 active voices, up to 32 pre-allocated)
   - Voice stealing algorithm (steals voice with lowest residual amplitude or oldest)
   - RT-safe: All voices pre-allocated, no runtime allocations

3. **SynthesisEngine** (`SynthesisEngine.h/cpp`)
   - Orchestrates VoiceManager
   - Lock-free event queue for OSC triggers
   - Master limiter
   - Parameter propagation to active voices

4. **MainComponent** (`MainComponent.h/cpp`)
   - UI with sliders and indicators
   - OSC receiver (port 9000)
   - Audio callbacks

**Key Files:**
- `app-juce/app-JUCE-PAS1/Source/ModalVoice.h/cpp` - Voice implementation
- `app-juce/app-JUCE-PAS1/Source/VoiceManager.h/cpp` - Polyphony management
- `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.h/cpp` - Main synthesis engine
- `app-juce/app-JUCE-PAS1/Source/MainComponent.h/cpp` - UI and OSC handling

## Proposed Enhancements

Based on analysis, the following improvements are proposed to enhance timbral variety and expressiveness:

### Priority 1: Excitation Waveform Selection (HIGH IMPACT, LOW COST)

**Goal:** Add different waveform options for the excitation signal to create more timbral variety.

**Current Implementation:**
- Excitation is generated in `ModalVoice::generateExcitation()`
- Uses differentiated white noise (noise burst with high-pass)
- Duration: 4-8ms variable
- Stored in `excitationBuffer[128]`

**Proposed Waveforms:**
1. **Noise** (current) - Differentiated white noise
2. **Sine** - Smooth sinusoidal impulse
3. **Square** - Square wave impulse (more aggressive)
4. **Saw** - Sawtooth wave (brighter)
5. **Triangle** - Triangular wave (smooth)
6. **Click** - Delta impulse (very percussive)
7. **Pulse** - Narrow pulse (very sharp)

**Implementation Requirements:**
- Add enum `ExcitationWaveform` to `ModalVoice.h`
- Add waveform selector parameter
- Implement waveform generation methods
- Add UI control (dropdown or buttons) in `MainComponent`
- Maintain RT-safety (no allocations in audio thread)

### Priority 2: Sub-Oscillator (HIGH IMPACT, MEDIUM COST)

**Goal:** Add a sub-oscillator one octave below the base frequency to add body and weight to the sound.

**Proposed Implementation:**
- Sub-oscillator at `baseFreq / 2`
- Waveform: Square or Sine (selectable)
- Mix parameter: 0.0-1.0
- Add to `ModalVoice` class
- Sum with modal synthesis output

### Priority 3: Increase Modal Modes (HIGH IMPACT, MEDIUM COST)

**Goal:** Increase from 4 to 6 resonant modes for richer spectral content.

**Current:** `NUM_MODES = 4`
**Proposed:** `NUM_MODES = 6`

**Additional Inharmonic Factors Needed:**
- Mode 4: ~13.34
- Mode 5: ~18.65

### Priority 4: Adaptive Excitation (MEDIUM IMPACT, LOW COST)

**Goal:** Make excitation adapt to energy level (from OSC).

**Proposed:**
- High energy → more aggressive excitation (square/pulse)
- Low energy → softer excitation (sine/triangle)
- Energy-based waveform selection or mixing

### Priority 5: FM/AM Modulation (HIGH IMPACT, HIGH COST)

**Goal:** Add frequency and amplitude modulation for dynamic timbre.

**Proposed:**
- LFO for FM (vibrato effect)
- LFO for AM (tremolo effect)
- Parameters: rate, depth
- Modulate modal frequencies and/or amplitudes

## Constraints

1. **RT-Safety:** All audio processing must be RT-safe (no allocations, no locks in audio thread)
2. **CPU Usage:** Target < 40% CPU on development machine
3. **Compatibility:** Must work with existing OSC integration
4. **Code Style:** Follow JUCE conventions, maintain existing code structure

## Request

**Generate a detailed implementation plan** that includes:

1. **Phased approach** - Break down into manageable phases
2. **File modifications** - Specific files to modify with line references where possible
3. **Code structure** - Proposed class/function additions
4. **UI changes** - How to add controls to MainComponent
5. **Testing strategy** - How to validate each enhancement
6. **Performance considerations** - CPU impact analysis
7. **Dependencies** - Order of implementation (what must be done first)

**Focus Areas:**
- Start with Priority 1 (Excitation Waveforms) as it has the highest impact-to-cost ratio
- Provide detailed code examples for waveform generation
- Consider backward compatibility with existing OSC messages
- Maintain the metallic "coin cascade" character while adding variety

**Output Format:**
- Use markdown with code blocks
- Include mermaid diagrams if helpful for architecture
- Provide specific file paths and function signatures
- List todos/tasks in a structured format

Please create a comprehensive, actionable implementation plan that I can follow step-by-step.
