/*
  ==============================================================================

    Note.h
    Created: 28 Jul 2025
    Author:  Developer Agent

    Represents a single MIDI note event.

  ==============================================================================
*/

#pragma once

//==============================================================================
/**
 * Represents a single musical note with its essential MIDI properties.
 */
struct Note
{
    int pitch = 0;          // MIDI note number (0-127)
    int velocity = 100;     // MIDI velocity (0-127)
    double startTime = 0.0; // Start time in beats from the beginning of the pattern
    double duration = 1.0;  // Duration in beats
};
