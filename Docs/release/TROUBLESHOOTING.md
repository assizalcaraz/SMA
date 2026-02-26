# Troubleshooting PAS-1 and ISTR Demo

## No sound from PAS-1

### Test Trigger works, but no sound from ISTR (OSC)

- **OSC connection:** PAS-1 must be running first and show “OSC: Connected (port 9000)”. Start PAS-1 before Particles.
- **ISTR OSC target:** In the Particles app, set OSC host to `127.0.0.1` (or `localhost`) and port to `9000`.
- **Firewall:** Ensure no firewall rule is blocking local UDP port 9000.

### No sound at all (including Test Trigger)

- **Audio device:** In macOS, check **System Preferences → Sound → Output** and that the correct output device is selected and not muted.
- **App permissions:** Grant the PAS-1 app access to the microphone if the system prompts (needed for some audio setups).
- **Buffer size:** If the audio driver reports “HAL overload” or similar, increase the buffer size (see below).

## HAL overload / audio dropouts

- **Buffer size:** Use **256 or 512 samples** (avoid 64 or 128 if you get overloads). In macOS this is often set in **Audio MIDI Setup** or in the app’s audio settings if exposed.
- **Sample rate:** 44.1 kHz or 48 kHz is typical; very high rates with small buffers can overload the audio thread.
- **Background load:** Close other heavy audio/CPU applications while testing.

## Latency

- Larger buffer sizes (512, 1024) reduce overload risk but increase latency. For interactive demo, 256–512 is a reasonable compromise.
- If PAS-1 exposes a buffer size or block size setting, prefer 256–512 for stability.

## ISTR and PAS-1 not in sync

- Start PAS-1 first so it is listening before ISTR sends. Restart both if you changed OSC port or host.
- Check PAS-1’s “OSC Messages: X/s” and “Hit Coverage” to confirm that hits are being received and processed.
