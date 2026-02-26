# Build PAS-1 on macOS

## Requirements

- **Xcode** (full app, not just command-line tools): needed to build and run the JUCE application.
- macOS 10.14 or later (or as required by your Xcode version).

## Steps

1. Open the Xcode project:
   - Path: `app-juce/app-JUCE-PAS1/Builds/MacOSX/app-JUCE-PAS1.xcodeproj`
   - Double-click the `.xcodeproj` or open from Xcode **File → Open**.

2. Select the scheme **app-JUCE-PAS1** and build configuration **Debug** or **Release**.

3. Build: **Product → Build** (or ⌘B).

4. Run: **Product → Run** (or ⌘R). The PAS-1 window opens and listens for OSC on port **9000**.

## Regenerating the Xcode project (optional)

If you need to change modules or project settings via Projucer:

1. Open `app-juce/app-JUCE-PAS1/app-JUCE-PAS1.jucer` in **Projucer**.
2. Click **Save Project** to regenerate the Xcode project and JuceLibraryCode.
3. Open the regenerated `.xcodeproj` in Xcode and build as above.

## Output

- Debug build: `app-juce/app-JUCE-PAS1/Builds/MacOSX/build/Debug/app-JUCE-PAS1.app`
- Release build: `app-juce/app-JUCE-PAS1/Builds/MacOSX/build/Release/app-JUCE-PAS1.app`

These paths are typically ignored by git (see repository `.gitignore`).
