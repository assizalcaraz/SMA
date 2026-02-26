# Running the ISTR + PAS-1 Demo

This describes how to run the particle simulation (ISTR) and the PAS-1 synthesizer together so that hits from the simulation are sonified in real time.

## Order of startup

1. **Start PAS-1 first** (the synthesizer):
   - Build and run the app as in [BUILD_MACOS.md](BUILD_MACOS.md).
   - PAS-1 listens for OSC on **port 9000**.
   - In the PAS-1 window, check that the label shows **OSC: Connected (port 9000)** (green when active).

2. **Start ISTR (Particles)**:
   - Open and run the Particles project (Xcode or Make, depending on your repo layout).
   - In the Particles GUI, ensure **OSC is enabled** and the target is **localhost** (or `127.0.0.1`) on port **9000**.
   - If the Particles app has an “OSC Host” / “OSC Port” setting, set Host to `127.0.0.1` and Port to `9000`.

## Verifying the link

- In PAS-1, the **OSC Messages** counter (e.g. “OSC Messages: X/s”) should increase when particles generate hits in ISTR.
- Use the **Test Trigger** button in PAS-1 to confirm that the synthesizer produces sound without OSC (then move particles in ISTR to confirm OSC-driven sound).

## Tips

- If no sound arrives from ISTR, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md) (OSC, audio device, buffer size).
- For a quick A/B test of presets: change the **Preset** in PAS-1, press **Test Trigger** three times, and compare the sound (see [PRESETS.md](PRESETS.md)).
