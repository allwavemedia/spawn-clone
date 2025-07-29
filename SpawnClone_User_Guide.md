# SpawnClone Plugin User Guide
## AI-Powered MIDI Pattern Generator

**Version:** 1.0  
**Date:** July 29, 2025  
**Platform:** VST3 / AudioUnit / Standalone  
**Compatibility:** Windows, macOS, Linux  

---

## Table of Contents

1. [Quick Start Guide](#quick-start-guide)
2. [Interface Overview](#interface-overview)
3. [Parameter Controls](#parameter-controls)
4. [Pattern Generation](#pattern-generation)
5. [Pattern Visualization](#pattern-visualization)
6. [Pattern History & Management](#pattern-history--management)
7. [Audio Preview System](#audio-preview-system)
8. [MIDI Export & DAW Integration](#midi-export--daw-integration)
9. [Workflow Examples](#workflow-examples)
10. [Troubleshooting](#troubleshooting)
11. [Keyboard Shortcuts](#keyboard-shortcuts)
12. [Advanced Features](#advanced-features)

---

## Quick Start Guide

### Installation

#### **Release Installation** (Recommended)
SpawnClone v1.0.0 now includes professional installers for easy setup:

1. **Download SpawnClone_v1.0.0_macOS.dmg** from the releases page
2. **Open the DMG file** by double-clicking
3. **For Standalone Use:**
   - Drag `SpawnClone.app` to your Applications folder
   - Launch from Applications or Launchpad
4. **For DAW Plugin Use:**
   - Double-click `Install Plugins.command` 
   - Follow the prompts to install VST3 and AudioUnit plugins
   - Restart your DAW and scan for new plugins

#### **Development Build Installation** (Advanced Users)
For developers or those building from source:

1. **Build Location:** After building the project, plugin files are located in:
   ```
   /Users/laurencedoby/Desktop/Development/spawn-clone/build/SpawnClone_artefacts/
   ```

2. **Manual Installation:**
   - **Standalone:** Copy `Standalone/SpawnClone.app` to `/Applications/`
   - **macOS VST3:** Copy `VST3/SpawnClone.vst3` to `~/Library/Audio/Plug-Ins/VST3/`
   - **macOS AU:** Copy `AU/SpawnClone.component` to `~/Library/Audio/Plug-Ins/Components/`
   - **Windows VST3:** Copy `SpawnClone.vst3` to `%PROGRAMFILES%\Common Files\VST3\`
   - **Linux VST3:** Copy `SpawnClone.vst3` to `~/.vst3/`

3. **Launch your DAW and scan for new plugins**

### First Pattern Generation
1. Load SpawnClone in your DAW as a MIDI instrument/effect
2. Click the large **Generate** button in the center
3. Wait 1-3 seconds for AI processing to complete
4. Click the **Play** button to preview your generated pattern
5. Drag the pattern to your DAW's piano roll or MIDI track

---

## Interface Overview

SpawnClone features a clean, intuitive interface designed for both beginners and professionals:

### Main Layout (800x600 pixels)

```
┌─────────────────────────────────────────────────────────────────┐
│                    SpawnClone v1.0                             │
├─────────────────────┬───────────────────────┬─────────────────────┤
│                     │                       │                     │
│   Parameter         │    Pattern Display    │   Pattern History   │
│   Controls          │      (Piano Roll)     │     & Favorites     │
│                     │                       │                     │
│  • Key: C          │  ████ ██  ████       │  ♪ Melody_001       │
│  • Scale: Major     │  ████ ██  ████       │  ♪ Chord_002        │
│  • Tempo: 120       │  ████ ██  ████       │  ⭐ Bass_003        │
│  • Complexity: 50%  │  ████ ██  ████       │  ♪ Drum_004         │
│  • Type: Melody     │                       │                     │
│                     │                       │                     │
│  [GENERATE]         │  Playhead: ▶         │  [Clear History]    │
│                     │                       │                     │
├─────────────────────┼───────────────────────┼─────────────────────┤
│   Preview Controls  │     Export Options    │   Status & Info     │
│                     │                       │                     │
│  ▶ Play/Stop       │  [Drag to DAW]       │  Last Generated:    │
│  🔊 Volume: 70%    │  [Copy MIDI]         │  2.3 seconds        │
│  🎹 Piano/Saw/Bass  │  [Export File]       │  Pattern Length:    │
│                     │                       │  4 bars             │
└─────────────────────┴───────────────────────┴─────────────────────┘
```

### UI Components

#### 🎛️ **Parameter Control Panel** (Left Side)
- **Musical Parameters:** Key, Scale, Tempo, Complexity, Generation Type
- **Quick Presets:** Genre-based quick settings
- **Generate Button:** Large, prominent button for pattern creation

#### 🎹 **Pattern Display Panel** (Center)
- **Piano Roll View:** Visual representation of generated MIDI notes
- **Playback Indicator:** Red playhead shows current position during preview
- **Drag & Drop:** Direct dragging to DAW tracks
- **Note Information:** Hover for pitch, velocity, and timing details

#### 📝 **Pattern History Panel** (Right Side)
- **Recent Patterns:** List of last 50 generated patterns
- **Favorites System:** Star patterns you want to keep
- **Quick Access:** Double-click to load previous patterns
- **Pattern Metadata:** Shows generation parameters and timestamp

#### 🎵 **Preview Control Panel** (Bottom Left)
- **Transport Controls:** Play, stop, loop playback
- **Volume Control:** Master preview volume
- **Instrument Selection:** Choose preview sound (Piano, Synth, Bass, Drums)

---

## Parameter Controls

### 🎼 Musical Parameters

#### **Key Selection**
- **Options:** C, C#, D, D#, E, F, F#, G, G#, A, A#, B
- **Purpose:** Sets the root note for pattern generation
- **AI Impact:** Influences note selection and harmonic content
- **Tip:** Match your project's key for best results

#### **Scale Selection**
- **Major:** Bright, happy sound (C-D-E-F-G-A-B)
- **Minor:** Darker, emotional sound (C-D-Eb-F-G-Ab-Bb)
- **Pentatonic:** Simple, universally pleasant (C-D-E-G-A)
- **Blues:** Soulful, expressive (C-Eb-F-F#-G-Bb)
- **Dorian:** Modal, jazzy flavor (C-D-Eb-F-G-A-Bb)
- **Mixolydian:** Rock/folk sound (C-D-E-F-G-A-Bb)

#### **Tempo Control**
- **Range:** 60-200 BPM
- **Default:** 120 BPM
- **Function:** Sets the speed for pattern generation and preview
- **Sync:** Automatically syncs with DAW tempo when available

#### **Complexity Slider**
- **Range:** 0% - 100%
- **Low (0-30%):** Simple patterns, few notes, basic rhythms
- **Medium (30-70%):** Balanced complexity, good for most genres
- **High (70-100%):** Complex patterns, many notes, intricate rhythms
- **Real-time:** Changes affect the next generation

#### **Generation Type**
- **Melody:** Single-note patterns for lead instruments
- **Chords:** Harmonic patterns with multiple simultaneous notes
- **Bassline:** Low-register patterns optimized for bass instruments
- **Drums:** Percussion patterns (in development)

### 🎨 Quick Presets

Access genre-specific parameter combinations:

| Preset | Key | Scale | Tempo | Complexity | Type |
|--------|-----|-------|--------|------------|------|
| **Pop** | C | Major | 120 | 40% | Melody |
| **EDM** | Am | Minor | 128 | 70% | Chords |
| **Hip-Hop** | F | Minor | 90 | 60% | Bassline |
| **Jazz** | F | Dorian | 140 | 80% | Chords |
| **Rock** | E | Mixolydian | 130 | 50% | Melody |
| **Ambient** | C | Pentatonic | 80 | 30% | Chords |

---

## Pattern Generation

### 🚀 Generation Process

#### **Step 1: Set Parameters**
1. Choose your musical key and scale
2. Set the tempo (or sync with DAW)
3. Adjust complexity to taste
4. Select generation type (Melody/Chords/Bassline)

#### **Step 2: Generate Pattern**
1. Click the **GENERATE** button
2. AI processing begins (indicated by progress animation)
3. Wait 1-5 seconds depending on complexity
4. New pattern appears in the display panel

#### **Step 3: Review and Refine**
1. Use preview controls to hear the pattern
2. If unsatisfied, click **GENERATE** again for a new variation
3. Previous patterns are automatically saved to history
4. Star patterns you like for easy access

### ⚡ Generation Modes

#### **Fast Mode** (Default)
- **Processing Time:** 1-2 seconds
- **Quality:** Good for rapid iteration
- **Method:** Rule-based AI with ML enhancement
- **Best For:** Sketching ideas, live performance

#### **Quality Mode** (Coming in v1.1)
- **Processing Time:** 3-5 seconds
- **Quality:** Enhanced musical coherence
- **Method:** Full ML inference
- **Best For:** Final productions, complex arrangements

#### **Cloud Mode** (Premium Feature)
- **Processing Time:** 5-10 seconds
- **Quality:** State-of-the-art AI models
- **Method:** Server-based processing
- **Best For:** Professional productions

### 🎯 Generation Tips

#### **For Better Results:**
- Start with medium complexity (40-60%) and adjust
- Use the preview system to understand what works
- Try different scales with the same key for variations
- Generate multiple patterns and combine the best parts

#### **Genre-Specific Advice:**
- **Electronic Music:** Higher complexity, minor scales
- **Pop Music:** Major scales, moderate complexity
- **Ambient:** Lower complexity, extended scales (Dorian, Mixolydian)
- **Jazz:** High complexity, modal scales

#### **Creative Techniques:**
- Generate in one key, then transpose in your DAW
- Use melody generator for both lead and harmony parts
- Combine multiple patterns for layered arrangements
- Use bassline mode for percussive elements

---

## Pattern Visualization

### 🎹 Piano Roll Display

The Pattern Display Panel shows your generated MIDI in a familiar piano roll format:

#### **Visual Elements:**

**Notes**
- **Blue Rectangles:** Individual MIDI notes
- **Height:** Represents pitch (higher = higher pitch)
- **Width:** Represents note duration
- **Intensity:** Brightness indicates velocity (louder = brighter)

**Grid System**
- **Vertical Lines:** Beat divisions (1/4, 1/8, 1/16 notes)
- **Horizontal Lines:** Piano keys (C, C#, D, etc.)
- **Range:** C2 to C7 (5 octaves)

**Playback Indicator**
- **Red Line:** Current playback position
- **Animation:** Moves smoothly during preview playback
- **Sync:** Follows DAW transport when playing

#### **Interactive Features:**

**Hover Information**
- Move mouse over any note to see:
  - Pitch name (e.g., "C4")
  - Velocity (0-127)
  - Start time (beats)
  - Duration (beats)

**Visual Settings**
- **Zoom:** Scroll to zoom in/out horizontally
- **Note Height:** Adjustable for better visibility
- **Color Coding:** Different colors for different generation types

#### **Understanding the Display:**

```
Pitch    Piano Roll View
─────    ────────────────────────────────────────
C5  │    ████                    ████
B4  │         ██              ██
A#4 │
A4  │              ████    ████
G#4 │
G4  │    ████              ████
F#4 │         ██      ██
F4  │              ████
E4  │
D#4 │
D4  │    ████                    ████
C#4 │
C4  │         ████        ████
─────    ────────────────────────────────────────
Time:    1    2    3    4    1    2    3    4
         Beat Markers (4/4 time)
```

### 📊 Pattern Analysis

The display also provides visual feedback about pattern characteristics:

#### **Pattern Statistics** (Bottom info bar)
- **Note Count:** Total number of notes
- **Density:** Notes per beat
- **Range:** Pitch range (lowest to highest note)
- **Complexity Score:** Calculated rhythmic complexity

#### **Visual Quality Indicators**
- **Green Highlights:** Well-placed notes (strong beats)
- **Yellow Highlights:** Syncopated notes (interesting rhythm)
- **Blue Highlights:** Harmonic notes (chord tones)

---

## Pattern History & Management

### 📚 History Panel Features

#### **Automatic History**
- **Capacity:** Stores last 50 generated patterns
- **Auto-Save:** Every generated pattern is automatically saved
- **Persistence:** History survives plugin reload
- **Metadata:** Each entry includes generation parameters and timestamp

#### **Pattern List Display**
```
♪ Melody_001     [⭐]    Jul 29, 14:23
♪ Chord_002      [ ]    Jul 29, 14:20
♪ Bass_003       [⭐]    Jul 29, 14:18
♪ Melody_004     [ ]    Jul 29, 14:15
♪ Chord_005      [ ]    Jul 29, 14:12
─────────────────────────────────────
[ Clear History ]     [ Export All ]
```

#### **Pattern Information**
Each history item shows:
- **🎵 Type Icon:** Melody (♪), Chords (♫), Bassline (♩)
- **Name:** Auto-generated descriptive name
- **⭐ Favorite Star:** Click to favorite/unfavorite
- **Timestamp:** When the pattern was generated
- **Parameters:** Hover to see generation settings

### ⭐ Favorites System

#### **Adding Favorites**
- Click the star icon next to any pattern
- Favorite patterns appear at the top of the list
- Maximum 20 favorites (oldest automatically removed)

#### **Managing Favorites**
- **Quick Access:** Favorites always visible at top
- **Export:** Batch export all favorites
- **Organize:** Drag to reorder favorites
- **Backup:** Export favorites as a collection

### 🔄 Pattern Operations

#### **Loading Previous Patterns**
- **Single Click:** Preview the pattern
- **Double Click:** Load pattern into main display
- **Right Click:** Context menu with additional options

#### **Pattern Context Menu**
- **Load Pattern:** Set as current pattern
- **Preview Pattern:** Quick audio preview
- **Add to Favorites:** Toggle favorite status
- **Export MIDI:** Save individual pattern as MIDI file
- **Copy to Clipboard:** Copy MIDI data
- **Delete Pattern:** Remove from history
- **Show Details:** View generation parameters

#### **Bulk Operations**
- **Select Multiple:** Ctrl/Cmd + click to select multiple patterns
- **Export Selected:** Save multiple patterns as MIDI files
- **Clear Selection:** ESC key to deselect all
- **Delete Selected:** Remove multiple patterns at once

### 📤 History Export

#### **Export Options**
- **Individual MIDI Files:** Each pattern as separate .mid file
- **Combined MIDI File:** All patterns in one file with track separation
- **SpawnClone Project:** Save parameters and patterns for later import
- **Pattern Library:** Export as reusable preset collection

---

## Audio Preview System

### 🔊 Preview Controls

#### **Transport Controls**
- **▶ Play Button:** Start/stop preview playback
- **⏸ Stop Button:** Stop and return to beginning
- **🔁 Loop Button:** Continuously loop the pattern
- **⏩ Fast Forward:** Skip to end of pattern

#### **Volume Control**
- **Range:** 0% - 100%
- **Default:** 70%
- **Real-time:** Adjustable during playback
- **Memory:** Volume setting preserved between sessions

#### **Tempo Sync**
- **DAW Sync:** Automatically matches host tempo
- **Manual Override:** Set custom tempo when standalone
- **Real-time:** Changes apply immediately to playback

### 🎹 Instrument Selection

Choose the preview sound that best matches your intended use:

#### **Piano** (Default)
- **Use For:** Melody patterns, chord progressions
- **Character:** Clean, neutral sound for accurate pitch reference
- **Range:** Full 88-key range

#### **Synth Saw**
- **Use For:** Electronic music, lead melodies
- **Character:** Bright, cutting synthesizer sound
- **Range:** Optimized for C2-C7

#### **Synth Square**
- **Use For:** Retro sounds, chiptune styles
- **Character:** Classic square wave synthesis
- **Range:** Full range with vintage character

#### **Bass**
- **Use For:** Bassline patterns, low-frequency content
- **Character:** Deep, punchy bass sound
- **Range:** Optimized for C1-C4

#### **Electric Piano**
- **Use For:** Jazz, funk, soul patterns
- **Character:** Warm, bell-like electric piano
- **Range:** Full range with authentic character

### 🎚️ Advanced Preview Settings

#### **Audio Quality**
- **Sample Rate:** 44.1kHz/48kHz/96kHz
- **Buffer Size:** Adjustable for latency vs. stability
- **Bit Depth:** 16/24-bit options

#### **MIDI Settings**
- **Velocity Curve:** Linear, exponential, logarithmic
- **Timing Humanization:** Add subtle timing variations
- **Velocity Humanization:** Add velocity variations

#### **Effects** (Coming in v1.1)
- **Reverb:** Add spatial depth
- **Delay:** Add rhythmic echoes
- **Filter:** Shape the tone
- **Compression:** Even out dynamics

---

## MIDI Export & DAW Integration

### 🎯 Drag & Drop to DAW

#### **Direct Drag & Drop**
1. Generate or select a pattern
2. Click and hold on the pattern display
3. Drag to your DAW's MIDI track or piano roll
4. Release to drop the MIDI data

#### **Supported DAWs**
- **Full Support:** Ableton Live, Logic Pro, Cubase, FL Studio, Reaper
- **Partial Support:** Pro Tools, Studio One, Reason
- **MIDI File Support:** All DAWs that accept MIDI files

#### **Drag & Drop Tips**
- Ensure your DAW track is armed for recording
- Some DAWs require dropping on the arrangement view
- Hold Ctrl/Cmd while dragging for copy (preserves original)

### 💾 MIDI File Export

#### **Export Current Pattern**
1. Right-click on the pattern display
2. Select "Export as MIDI File"
3. Choose destination and filename
4. Click Save

#### **Export Options**
- **Standard MIDI File:** Compatible with all software
- **Type 0:** Single track (recommended)
- **Type 1:** Multiple tracks (for complex patterns)
- **Resolution:** 480 PPQ (ticks per quarter note)

#### **File Naming Convention**
```
SpawnClone_[Type]_[Key][Scale]_[Tempo]BPM_[Timestamp].mid

Examples:
SpawnClone_Melody_CMajor_120BPM_20250729_142330.mid
SpawnClone_Chords_AmMinor_140BPM_20250729_143045.mid
SpawnClone_Bass_FDorian_90BPM_20250729_143512.mid
```

### 📋 Clipboard Integration

#### **Copy MIDI Data**
- **Method 1:** Ctrl/Cmd + C with pattern selected
- **Method 2:** Right-click → "Copy to Clipboard"
- **Method 3:** Edit menu → Copy Pattern

#### **Paste into DAW**
1. Copy pattern to clipboard
2. Navigate to your DAW's MIDI editor
3. Paste with Ctrl/Cmd + V
4. Pattern appears at cursor position

#### **Clipboard Format**
- Standard MIDI event data
- Compatible with most DAW clipboard systems
- Preserves timing, velocity, and pitch information

### 🔄 DAW Synchronization

#### **Tempo Sync**
- **Automatic:** Reads tempo from DAW when available
- **Real-time:** Updates when DAW tempo changes
- **Override:** Manual tempo setting for standalone use

#### **Transport Sync**
- **Play/Stop:** Follows DAW transport
- **Position:** Syncs playback position
- **Record:** Can trigger generation on DAW record

#### **Parameter Automation**
- **All Parameters:** Can be automated from DAW
- **Real-time:** Changes apply immediately
- **Smooth:** Interpolated parameter changes

---

## Workflow Examples

### 🎵 Example 1: Creating a Pop Song Foundation

#### **Goal:** Create a complete 4-bar pop progression

**Step 1: Generate Chord Progression**
1. Set Key: **C Major**
2. Set Scale: **Major**
3. Set Type: **Chords**
4. Set Complexity: **40%**
5. Click **Generate**
6. Preview and save to favorites

**Step 2: Create Melody**
1. Change Type to: **Melody**
2. Increase Complexity to: **60%**
3. Click **Generate**
4. Preview with chord progression in mind
5. Regenerate until you get a complementary melody

**Step 3: Add Bassline**
1. Change Type to: **Bassline**
2. Set Complexity: **30%**
3. Click **Generate**
4. Look for patterns that support the chord progression

**Step 4: Export to DAW**
1. Drag chord progression to Track 1
2. Drag melody to Track 2  
3. Drag bassline to Track 3
4. Arrange and layer in your DAW

### 🎹 Example 2: Jazz Chord Exploration

#### **Goal:** Explore modern jazz harmony

**Step 1: Set Jazz Parameters**
1. Key: **F**
2. Scale: **Dorian**
3. Type: **Chords**
4. Complexity: **80%**
5. Tempo: **140 BPM**

**Step 2: Generate Variations**
1. Generate first pattern
2. Add to favorites if interesting
3. Click **Generate** repeatedly
4. Save 5-6 different progressions

**Step 3: Analyze and Learn**
1. Export favorites as MIDI
2. Import into notation software
3. Analyze chord progressions
4. Use as study material

### 🎧 Example 3: Electronic Music Production

#### **Goal:** Create atmospheric electronic elements

**Step 1: Ambient Pad Chords**
1. Key: **A Minor**
2. Scale: **Minor**
3. Type: **Chords**
4. Complexity: **25%**
5. Preview with **Synth Saw** instrument

**Step 2: Arpeggiated Lead**
1. Keep same key/scale
2. Change Type to: **Melody**
3. Increase Complexity to: **70%**
4. Generate until you get flowing patterns

**Step 3: Sub Bass**
1. Change Type to: **Bassline**
2. Set Complexity: **20%**
3. Preview with **Bass** instrument
4. Look for simple, repetitive patterns

**Step 4: Layer in DAW**
1. Import all patterns to separate tracks
2. Add synthesis and effects
3. Arrange for dynamic build-up

### 🎸 Example 4: Song Arrangement Ideas

#### **Goal:** Break through writer's block

**Step 1: Random Inspiration**
1. Use **Random** preset button
2. Generate 10 different patterns
3. Don't overthink - just generate
4. Save anything that catches your ear

**Step 2: Pattern Analysis**
1. Review saved patterns
2. Identify interesting rhythmic or melodic motifs
3. Note which keys/scales work well
4. Look for unexpected combinations

**Step 3: Development**
1. Take one interesting pattern
2. Use it as foundation
3. Generate variations in related keys
4. Build arrangement around the core idea

---

## Troubleshooting

### ⚠️ Common Issues

#### **Plugin Won't Load**
**Symptoms:** Plugin not visible in DAW
**Solutions:**
- Ensure plugin is installed in correct directory
- Rescan plugins in your DAW
- Check plugin format compatibility (VST3/AU)
- Verify system requirements are met

#### **No Sound During Preview**
**Symptoms:** Pattern generates but no audio output
**Solutions:**
- Check preview volume setting
- Verify audio device settings
- Ensure DAW audio is working
- Try different instrument preset

#### **Generation Takes Too Long**
**Symptoms:** Generate button shows loading for >10 seconds
**Solutions:**
- Reduce complexity setting
- Close other resource-intensive applications
- Check available RAM (requires minimum 4GB)
- Try restarting the plugin

#### **Patterns Don't Match Key/Scale**
**Symptoms:** Generated notes don't fit selected parameters
**Solutions:**
- Verify key and scale settings
- Check if custom scale is properly configured
- Try regenerating with different complexity
- Report bug with specific parameters used

#### **Drag & Drop Not Working**
**Symptoms:** Cannot drag patterns to DAW
**Solutions:**
- Ensure DAW track is armed for MIDI
- Try drag to different DAW areas (arrangement vs. piano roll)
- Use Export MIDI instead of drag & drop
- Check DAW-specific drag & drop requirements

### 🔧 Performance Optimization

#### **Improve Generation Speed**
- Close unnecessary applications
- Increase available RAM
- Use lower complexity settings for faster generation
- Enable GPU acceleration if available

#### **Reduce CPU Usage**
- Disable preview when not needed
- Lower audio buffer size in DAW
- Use fewer simultaneous instances
- Close pattern history when not needed

#### **Audio Quality Issues**
- Check sample rate compatibility
- Increase audio buffer size for stability
- Update audio drivers
- Use ASIO drivers on Windows

### 📞 Getting Help

#### **Built-in Help**
- **F1 Key:** Open help documentation
- **Tooltips:** Hover over controls for quick help
- **Status Bar:** Shows current operation status

#### **Online Resources**
- **User Manual:** Complete PDF documentation
- **Video Tutorials:** Step-by-step walkthroughs
- **Community Forum:** User questions and answers
- **FAQ:** Common questions and solutions

#### **Technical Support**
- **Email:** support@spawnclone.com
- **Response Time:** 24-48 hours
- **Include:** Version number, operating system, DAW information
- **Logs:** Available in plugin's debug menu

---

## Keyboard Shortcuts

### 🎹 Global Shortcuts

| Action | Windows/Linux | macOS | Description |
|--------|---------------|-------|-------------|
| Generate Pattern | **Space** | **Space** | Trigger new generation |
| Play/Stop Preview | **Enter** | **Return** | Toggle playback |
| Copy Pattern | **Ctrl + C** | **Cmd + C** | Copy to clipboard |
| Paste Pattern | **Ctrl + V** | **Cmd + V** | Paste from clipboard |
| Save to Favorites | **Ctrl + S** | **Cmd + S** | Add current pattern to favorites |
| Clear History | **Ctrl + Shift + Del** | **Cmd + Shift + Del** | Clear all history |
| Export MIDI | **Ctrl + E** | **Cmd + E** | Export current pattern |
| Focus Search | **Ctrl + F** | **Cmd + F** | Search pattern history |
| Help | **F1** | **F1** | Open help documentation |
| Settings | **Ctrl + ,** | **Cmd + ,** | Open preferences |

### 🎛️ Parameter Shortcuts

| Parameter | Shortcut | Action |
|-----------|----------|---------|
| Key | **1-12** | Select key (C=1, C#=2, etc.) |
| Scale | **Shift + 1-6** | Select scale type |
| Tempo | **↑/↓** | Increase/decrease by 5 BPM |
| Complexity | **Shift + ↑/↓** | Increase/decrease by 10% |
| Generation Type | **Tab** | Cycle through types |
| Random Parameters | **R** | Randomize all parameters |

### 📝 History Panel Shortcuts

| Action | Shortcut | Description |
|--------|----------|-------------|
| Select Next | **↓** | Move selection down |
| Select Previous | **↑** | Move selection up |
| Toggle Favorite | **F** | Add/remove from favorites |
| Load Pattern | **Enter** | Load selected pattern |
| Preview Pattern | **Space** | Quick preview |
| Delete Pattern | **Delete** | Remove from history |
| Multi-Select | **Ctrl/Cmd + Click** | Select multiple patterns |
| Select All | **Ctrl/Cmd + A** | Select all patterns |

### 🎵 Preview Shortcuts

| Action | Shortcut | Description |
|--------|----------|-------------|
| Play/Pause | **Space** | Toggle playback |
| Stop | **Escape** | Stop and return to start |
| Loop Toggle | **L** | Enable/disable looping |
| Volume Up | **+ (Plus)** | Increase preview volume |
| Volume Down | **- (Minus)** | Decrease preview volume |
| Next Instrument | **I** | Cycle through instruments |
| Mute Preview | **M** | Mute/unmute preview |

---

## Advanced Features

### 🤖 AI Enhancement Settings

#### **Generation Modes** (Pro Version)
- **Fast Mode:** Rule-based generation (1-2 seconds)
- **Quality Mode:** ML-enhanced generation (3-5 seconds)
- **Creative Mode:** Experimental AI algorithms (5-10 seconds)
- **Custom Mode:** User-defined AI parameters

#### **AI Model Selection**
- **Classical:** Traditional music theory rules
- **Jazz:** Advanced harmonic concepts  
- **Electronic:** Modern production techniques
- **World:** Global music traditions
- **Experimental:** Cutting-edge AI research

#### **Learning System** (Coming v1.2)
- **Style Learning:** AI learns from your favorite patterns
- **Personal Model:** Custom AI trained on your preferences
- **Collaboration:** Share learned styles with other users
- **Evolution:** Models improve with use over time

### 🎨 Custom Scales and Modes

#### **Scale Editor**
1. Open **Settings** → **Custom Scales**
2. Click **New Scale**
3. Define intervals from root note
4. Name your scale
5. Save and use in generation

#### **Predefined Exotic Scales**
- **Harmonic Minor:** Natural minor with raised 7th
- **Melodic Minor:** Ascending and descending variations
- **Whole Tone:** Six-note scale of whole steps
- **Diminished:** Alternating whole and half steps
- **Chromatic:** All 12 pitches
- **Pentatonic Minor:** Five-note minor scale

#### **Microtonal Support** (Experimental)
- **Quarter-tone scales:** 24-note equal temperament
- **Just intonation:** Pure interval ratios
- **Custom tunings:** User-defined pitch systems
- **Export options:** Special MIDI formats for microtonal DAWs

### 🔄 Pattern Manipulation

#### **Pattern Transformations**
- **Transpose:** Shift pitch up/down
- **Invert:** Mirror around axis
- **Retrograde:** Reverse time order
- **Augmentation:** Stretch timing
- **Diminution:** Compress timing
- **Rotation:** Rotate note positions

#### **Rhythmic Tools**
- **Quantize:** Snap to grid
- **Humanize:** Add timing variations
- **Swing:** Apply swing rhythm
- **Groove Templates:** Apply rhythmic feels
- **Polyrhythm:** Generate complex rhythms

#### **Harmonic Tools**
- **Voice Leading:** Smooth chord progressions
- **Reharmonization:** Alternative chord choices
- **Modal Interchange:** Borrow from parallel modes
- **Chord Extensions:** Add 7ths, 9ths, etc.
- **Bass Movement:** Intelligent bass line creation

### 🌐 Cloud Integration (Premium)

#### **Cloud Generation**
- **Server Processing:** Access to powerful AI models
- **Style Libraries:** Download genre-specific models
- **Collaboration:** Share patterns with team members
- **Version History:** Track pattern evolution
- **Cross-Device Sync:** Access patterns anywhere

#### **Community Features**
- **Pattern Sharing:** Upload your best creations
- **Style Exchange:** Trade AI-learned styles
- **Challenges:** Weekly generation competitions
- **Forums:** Discuss techniques and share tips
- **Tutorials:** Video lessons from pro users

### 📊 Analytics and Insights

#### **Generation Statistics**
- **Success Rate:** Percentage of patterns you keep
- **Preference Analysis:** Your most-used parameters
- **Style Evolution:** How your taste changes over time
- **Productivity Metrics:** Patterns generated per session

#### **Musical Analysis**
- **Harmonic Analysis:** Chord function identification
- **Rhythmic Complexity:** Quantified rhythm metrics
- **Melodic Contour:** Shape and direction analysis
- **Voice Leading:** Smoothness measurements

#### **Usage Reports**
- **Time Tracking:** How long you spend generating
- **Feature Usage:** Which tools you use most
- **Efficiency Tips:** Suggestions for faster workflow
- **Learning Path:** Recommended areas for improvement

---

**End of User Guide**

---

## About SpawnClone

SpawnClone is an open-source AI-powered MIDI pattern generator designed to inspire creativity and accelerate music production. Built with modern C++ and the JUCE framework, it provides professional-quality pattern generation with an intuitive user interface.

**Development Team:** Open Source Audio Community  
**License:** MIT License  
**Contributing:** Visit our GitHub repository to contribute  
**Support:** Join our Discord community for help and discussion  

**Version:** 1.0.0  
**Release Date:** July 29, 2025  
**Last Updated:** July 29, 2025
