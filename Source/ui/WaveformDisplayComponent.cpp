#include "WaveformDisplayComponent.h"
#include <cmath>

//==============================================================================
WaveformDisplayComponent::WaveformDisplayComponent()
    : audioRingBuffer(2, bufferSize)
{
    // Initialize FFT processor for spectrum analysis (order 11 = 2^11 = 2048 samples)
    fftProcessor = std::make_unique<juce::dsp::FFT>(11);
    
    // Initialize Hann window for FFT
    for (int i = 0; i < bufferSize; ++i)
    {
        windowBuffer[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (bufferSize - 1)));
    }
    
    // Clear audio buffers
    waveformBuffer.fill(0.0f);
    spectrumBuffer.fill(0.0f);
    phaseBuffer.fill(0.0f);
    fftBuffer.fill(0.0f);
    
    // Initialize wavetable with sine wave
    wavetableData.resize(maxSamples);
    for (int i = 0; i < maxSamples; ++i)
    {
        wavetableData[i] = std::sin(2.0f * juce::MathConstants<float>::pi * i / maxSamples);
    }
    
    // Start timer for real-time updates (60 FPS target)
    startTimer(1000 / settings.refreshRate);
    
    // Enable mouse events
    setMouseClickGrabsKeyboardFocus(true);
    
    DBG("WaveformDisplayComponent initialized with Epic 7 integration support");
}

WaveformDisplayComponent::~WaveformDisplayComponent()
{
    stopTimer();
}

//==============================================================================
void WaveformDisplayComponent::paint(juce::Graphics& g)
{
    auto startTime = juce::Time::getMillisecondCounterHiRes();
    
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.fillAll(settings.backgroundColour);
    
    // Update cached background if needed
    if (backgroundNeedsUpdate || lastBounds != getLocalBounds())
    {
        cachedBackground = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics bg(cachedBackground);
        
        if (settings.showGrid)
        {
            drawGrid(bg, bounds);
        }
        
        backgroundNeedsUpdate = false;
        lastBounds = getLocalBounds();
    }
    
    // Draw cached background
    g.drawImageAt(cachedBackground, 0, 0);
    
    // Render based on current display mode
    switch (settings.mode)
    {
        case Oscilloscope:
            renderOscilloscope(g, bounds);
            break;
            
        case Spectrum:
            renderSpectrum(g, bounds);
            break;
            
        case Phase:
            renderPhase(g, bounds);
            break;
            
        case Harmonic:
            renderHarmonic(g, bounds);
            break;
            
        case Wavetable:
            renderWavetable(g, bounds);
            break;
            
        case Lissajous:
            renderLissajous(g, bounds);
            break;
    }
    
    // Draw labels and cursor
    if (settings.showLabels)
    {
        drawLabels(g, bounds);
    }
    
    drawCursor(g, bounds);
    
    // Update performance stats
    auto endTime = juce::Time::getMillisecondCounterHiRes();
    performanceStats.renderTime = static_cast<float>(endTime - startTime);
    updatePerformanceStats();
}

void WaveformDisplayComponent::resized()
{
    backgroundNeedsUpdate = true;
}

//==============================================================================
void WaveformDisplayComponent::mouseDown(const juce::MouseEvent& event)
{
    if (settings.mode == Wavetable && isWavetableEditingEnabled)
    {
        handleWavetableMouseDown(event, getLocalBounds().toFloat());
    }
}

void WaveformDisplayComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (settings.mode == Wavetable && isWavetableEditingEnabled)
    {
        handleWavetableMouseDrag(event, getLocalBounds().toFloat());
    }
}

void WaveformDisplayComponent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (event.mods.isShiftDown())
    {
        // Amplitude zoom
        settings.amplitudeScale *= (1.0f + wheel.deltaY * 0.1f);
        settings.amplitudeScale = juce::jlimit(0.1f, 10.0f, settings.amplitudeScale);
    }
    else
    {
        // Time zoom
        settings.timeScale *= (1.0f + wheel.deltaY * 0.1f);
        settings.timeScale = juce::jlimit(0.1f, 10.0f, settings.timeScale);
    }
    
    repaint();
}

//==============================================================================
void WaveformDisplayComponent::timerCallback()
{
    if (shouldSkipFrame())
    {
        performanceStats.droppedFrames++;
        return;
    }
    
    // Update audio data from ring buffer
    updateWaveformData();
    
    // Update display mode specific data
    switch (settings.mode)
    {
        case Spectrum:
        case Harmonic:
            updateSpectrumData();
            break;
            
        case Phase:
        case Lissajous:
            updatePhaseData();
            break;
            
        default:
            break;
    }
    
    // Trigger detection for oscilloscope
    if (settings.mode == Oscilloscope && settings.triggerMode != None)
    {
        detectTrigger();
    }
    
    repaint();
    frameCount++;
}

//==============================================================================
void WaveformDisplayComponent::pushAudioData(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin(buffer.getNumChannels(), audioRingBuffer.getNumChannels());
    
    // Copy audio data to ring buffer
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* sourceData = buffer.getReadPointer(channel);
        float* destData = audioRingBuffer.getWritePointer(channel);
        
        for (int i = 0; i < numSamples; ++i)
        {
            int writePos = (writePosition + i) % bufferSize;
            destData[writePos] = sourceData[i];
        }
    }
    
    writePosition = (writePosition + numSamples) % bufferSize;
}

//==============================================================================
void WaveformDisplayComponent::setDisplayMode(DisplayMode newMode)
{
    if (settings.mode != newMode)
    {
        settings.mode = newMode;
        backgroundNeedsUpdate = true;
        repaint();
        
        DBG("Waveform display mode changed to: " << static_cast<int>(newMode));
    }
}

void WaveformDisplayComponent::setVisualizationSettings(const VisualizationSettings& newSettings)
{
    settings = newSettings;
    backgroundNeedsUpdate = true;
    
    // Update timer frequency if refresh rate changed
    if (isTimerRunning())
    {
        stopTimer();
        startTimer(1000 / settings.refreshRate);
    }
    
    repaint();
}

//==============================================================================
void WaveformDisplayComponent::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Epic 7 integration - respond to AI parameter changes
    if (parameterID.startsWith("ai_"))
    {
        // Update visualization based on AI parameters
        repaint();
    }
}

//==============================================================================
void WaveformDisplayComponent::renderOscilloscope(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    if (waveformBuffer.empty())
        return;
    
    // Create path for waveform
    juce::Path waveformPath;
    bool isFirstPoint = true;
    
    const float centerY = bounds.getCentreY();
    const float amplitude = bounds.getHeight() * 0.4f * settings.amplitudeScale;
    
    // Determine sample range based on time scale
    const int totalSamples = static_cast<int>(maxSamples / settings.timeScale);
    const int startSample = triggerDetected ? triggerPosition : 0;
    
    for (int i = 0; i < totalSamples && i < maxSamples; ++i)
    {
        const float x = bounds.getX() + (static_cast<float>(i) / totalSamples) * bounds.getWidth();
        const float y = centerY - waveformBuffer[(startSample + i) % maxSamples] * amplitude;
        
        if (isFirstPoint)
        {
            waveformPath.startNewSubPath(x, y);
            isFirstPoint = false;
        }
        else
        {
            waveformPath.lineTo(x, y);
        }
    }
    
    // Draw waveform with glow effect if enabled
    if (settings.enableGlow)
    {
        // Draw glow effect
        g.setColour(settings.waveformColour.withAlpha(0.3f));
        g.strokePath(waveformPath, juce::PathStrokeType(4.0f));
        
        g.setColour(settings.waveformColour.withAlpha(0.6f));
        g.strokePath(waveformPath, juce::PathStrokeType(2.0f));
    }
    
    // Draw main waveform
    g.setColour(settings.waveformColour);
    g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
}

void WaveformDisplayComponent::renderSpectrum(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    if (spectrumBuffer.empty())
        return;
    
    const float binWidth = bounds.getWidth() / (maxSamples / 2);
    const float maxHeight = bounds.getHeight() * 0.8f;
    
    // Draw spectrum bars
    for (int i = 1; i < maxSamples / 2; ++i)
    {
        const float magnitude = spectrumBuffer[i];
        const float x = bounds.getX() + i * binWidth;
        const float height = magnitude * maxHeight * settings.amplitudeScale;
        const float y = bounds.getBottom() - height;
        
        // Color based on frequency
        const float hue = static_cast<float>(i) / (maxSamples / 2);
        const juce::Colour barColour = juce::Colour::fromHSV(hue * 0.7f, 0.8f, magnitude * 0.8f + 0.2f, 1.0f);
        
        g.setColour(barColour);
        g.fillRect(x, y, binWidth * 0.8f, height);
        
        // Add glow effect for stronger components
        if (magnitude > 0.5f && settings.enableGlow)
        {
            g.setColour(barColour.withAlpha(0.3f));
            g.fillRect(x - 1, y - 1, binWidth * 0.8f + 2, height + 2);
        }
    }
}

void WaveformDisplayComponent::renderWavetable(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    if (wavetableData.empty())
        return;
    
    // Create path for wavetable
    juce::Path wavetablePath;
    const float centerY = bounds.getCentreY();
    const float amplitude = bounds.getHeight() * 0.4f * settings.amplitudeScale;
    
    for (size_t i = 0; i < wavetableData.size(); ++i)
    {
        const float x = bounds.getX() + (static_cast<float>(i) / wavetableData.size()) * bounds.getWidth();
        const float y = centerY - wavetableData[i] * amplitude;
        
        if (i == 0)
            wavetablePath.startNewSubPath(x, y);
        else
            wavetablePath.lineTo(x, y);
    }
    
    // Draw wavetable
    g.setColour(settings.waveformColour);
    g.strokePath(wavetablePath, juce::PathStrokeType(2.0f));
    
    // Draw edit points if editing is enabled
    if (isWavetableEditingEnabled)
    {
        const int pointSpacing = wavetableData.size() / 32; // 32 edit points
        
        for (int i = 0; i < wavetableData.size(); i += pointSpacing)
        {
            const float x = bounds.getX() + (static_cast<float>(i) / wavetableData.size()) * bounds.getWidth();
            const float y = centerY - wavetableData[i] * amplitude;
            
            juce::Colour pointColour = (i == selectedWavetablePoint) ? 
                juce::Colours::yellow : settings.waveformColour.brighter(0.5f);
            
            g.setColour(pointColour);
            g.fillEllipse(x - 3, y - 3, 6, 6);
        }
    }
}

//==============================================================================
void WaveformDisplayComponent::drawGrid(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    g.setColour(settings.gridColour);
    
    // Vertical grid lines (time)
    const int numVerticalLines = 10;
    for (int i = 1; i < numVerticalLines; ++i)
    {
        const float x = bounds.getX() + (static_cast<float>(i) / numVerticalLines) * bounds.getWidth();
        g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
    }
    
    // Horizontal grid lines (amplitude)
    const int numHorizontalLines = 8;
    for (int i = 1; i < numHorizontalLines; ++i)
    {
        const float y = bounds.getY() + (static_cast<float>(i) / numHorizontalLines) * bounds.getHeight();
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
    }
    
    // Center lines
    g.setColour(settings.gridColour.brighter(0.3f));
    g.drawHorizontalLine(static_cast<int>(bounds.getCentreY()), bounds.getX(), bounds.getRight());
    g.drawVerticalLine(static_cast<int>(bounds.getCentreX()), bounds.getY(), bounds.getBottom());
}

void WaveformDisplayComponent::drawLabels(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font("Arial", 10.0f, juce::Font::plain));
    
    // Mode label
    juce::String modeText;
    switch (settings.mode)
    {
        case Oscilloscope: modeText = "OSCILLOSCOPE"; break;
        case Spectrum: modeText = "SPECTRUM"; break;
        case Phase: modeText = "PHASE"; break;
        case Harmonic: modeText = "HARMONIC"; break;
        case Wavetable: modeText = "WAVETABLE"; break;
        case Lissajous: modeText = "LISSAJOUS"; break;
    }
    
    g.drawText(modeText, bounds.getX() + 5, bounds.getY() + 5, 100, 15, juce::Justification::topLeft);
    
    // Performance stats (top right)
    juce::String perfText = juce::String::formatted("FPS: %.1f | Render: %.1fms", 
        performanceStats.actualFPS.load(), performanceStats.renderTime.load());
    g.drawText(perfText, bounds.getRight() - 150, bounds.getY() + 5, 145, 15, juce::Justification::topRight);
    
    // Scale info (bottom left)
    juce::String scaleText = juce::String::formatted("Time: %.1fx | Amp: %.1fx", 
        settings.timeScale, settings.amplitudeScale);
    g.drawText(scaleText, bounds.getX() + 5, bounds.getBottom() - 20, 120, 15, juce::Justification::bottomLeft);
}

void WaveformDisplayComponent::drawCursor(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    // Draw trigger level indicator for oscilloscope mode
    if (settings.mode == Oscilloscope && settings.triggerMode != None)
    {
        const float centerY = bounds.getCentreY();
        const float amplitude = bounds.getHeight() * 0.4f * settings.amplitudeScale;
        const float triggerY = centerY - settings.triggerLevel * amplitude;
        
        g.setColour(juce::Colours::orange.withAlpha(0.8f));
        g.drawHorizontalLine(static_cast<int>(triggerY), bounds.getX(), bounds.getRight());
        
        // Trigger indicator
        g.drawText("TRIG", bounds.getRight() - 30, triggerY - 7, 25, 14, juce::Justification::centred);
    }
}

//==============================================================================
void WaveformDisplayComponent::updateWaveformData()
{
    if (audioRingBuffer.getNumSamples() == 0)
        return;
    
    // Copy samples from ring buffer to waveform buffer
    const float* channelData = audioRingBuffer.getReadPointer(0);
    const int samplesToRead = juce::jmin(maxSamples, bufferSize);
    
    for (int i = 0; i < samplesToRead; ++i)
    {
        int readPos = (readPosition + i) % bufferSize;
        waveformBuffer[i] = channelData[readPos];
    }
    
    readPosition = (readPosition + samplesToRead / 4) % bufferSize; // Advance read position
}

void WaveformDisplayComponent::updateSpectrumData()
{
    // Copy waveform data to FFT buffer and apply window
    for (int i = 0; i < juce::jmin(maxSamples, bufferSize); ++i)
    {
        fftBuffer[i] = waveformBuffer[i] * windowBuffer[i];
    }
    
    // Zero-pad if necessary
    for (int i = maxSamples; i < bufferSize; ++i)
    {
        fftBuffer[i] = 0.0f;
    }
    
    // Perform FFT
    fftProcessor->performFrequencyOnlyForwardTransform(fftBuffer.data());
    
    // Convert to dB and store in spectrum buffer
    for (int i = 0; i < maxSamples / 2; ++i)
    {
        float magnitude = fftBuffer[i];
        float magnitudeDb = 20.0f * std::log10(magnitude + 1e-10f);
        spectrumBuffer[i] = juce::jmap(magnitudeDb, -60.0f, 0.0f, 0.0f, 1.0f);
    }
}

void WaveformDisplayComponent::detectTrigger()
{
    triggerDetected = false;
    
    for (int i = 1; i < maxSamples; ++i)
    {
        bool trigger = false;
        
        switch (settings.triggerMode)
        {
            case Rising:
                trigger = (lastSample < settings.triggerLevel && waveformBuffer[i] >= settings.triggerLevel);
                break;
                
            case Falling:
                trigger = (lastSample > settings.triggerLevel && waveformBuffer[i] <= settings.triggerLevel);
                break;
                
            case Auto:
                trigger = (std::abs(waveformBuffer[i]) > 0.1f); // Auto-trigger on significant signal
                break;
                
            default:
                break;
        }
        
        if (trigger)
        {
            triggerDetected = true;
            triggerPosition = i;
            break;
        }
        
        lastSample = waveformBuffer[i];
    }
}

//==============================================================================
void WaveformDisplayComponent::updatePerformanceStats()
{
    auto currentTime = juce::Time::getMillisecondCounterHiRes();
    
    if (lastFrameTime > 0)
    {
        auto deltaTime = currentTime - lastFrameTime;
        float fps = 1000.0f / static_cast<float>(deltaTime);
        performanceStats.actualFPS = fps;
    }
    
    lastFrameTime = currentTime;
}

bool WaveformDisplayComponent::shouldSkipFrame() const
{
    // Skip frame if render time is too high to maintain target FPS
    return performanceStats.renderTime.load() > (1000.0f / settings.refreshRate) * 1.5f;
}

//==============================================================================
void WaveformDisplayComponent::setAIGenerationCallback(std::function<void(const std::vector<float>&)> callback)
{
    aiGenerationCallback = std::move(callback);
    DBG("AI generation callback set for Epic 7 integration");
}

void WaveformDisplayComponent::visualizeAIPattern(const std::vector<float>& pattern)
{
    currentAIPattern = pattern;
    
    // Convert AI pattern to waveform visualization
    if (!pattern.empty() && pattern.size() <= maxSamples)
    {
        for (size_t i = 0; i < pattern.size(); ++i)
        {
            waveformBuffer[i] = pattern[i];
        }
        
        repaint();
        DBG("Visualizing AI-generated pattern with " << pattern.size() << " samples");
    }
}

//==============================================================================
void WaveformDisplayComponent::resetPerformanceStats()
{
    performanceStats.actualFPS = 0.0f;
    performanceStats.renderTime = 0.0f;
    performanceStats.droppedFrames = 0;
    performanceStats.bufferUnderruns = 0;
    frameCount = 0;
    lastFrameTime = 0;
}

//==============================================================================
// Placeholder implementations for missing methods
void WaveformDisplayComponent::renderPhase(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    // TODO: Implement phase visualization
    g.setColour(settings.waveformColour.withAlpha(0.5f));
    g.drawText("Phase Mode - Coming Soon", bounds, juce::Justification::centred);
}

void WaveformDisplayComponent::renderHarmonic(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    // TODO: Implement harmonic analysis visualization
    g.setColour(settings.waveformColour.withAlpha(0.5f));
    g.drawText("Harmonic Mode - Coming Soon", bounds, juce::Justification::centred);
}

void WaveformDisplayComponent::renderLissajous(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    // TODO: Implement Lissajous curve visualization
    g.setColour(settings.waveformColour.withAlpha(0.5f));
    g.drawText("Lissajous Mode - Coming Soon", bounds, juce::Justification::centred);
}

void WaveformDisplayComponent::updatePhaseData()
{
    // TODO: Implement phase data calculation
}

void WaveformDisplayComponent::handleWavetableMouseDown(const juce::MouseEvent& event, const juce::Rectangle<float>& bounds)
{
    selectedWavetablePoint = findNearestWavetablePoint(event.position, bounds);
}

void WaveformDisplayComponent::handleWavetableMouseDrag(const juce::MouseEvent& event, const juce::Rectangle<float>& bounds)
{
    if (selectedWavetablePoint >= 0)
    {
        const float centerY = bounds.getCentreY();
        const float amplitude = bounds.getHeight() * 0.4f * settings.amplitudeScale;
        const float newValue = (centerY - event.position.y) / amplitude;
        
        wavetableData[selectedWavetablePoint] = juce::jlimit(-1.0f, 1.0f, newValue);
        repaint();
    }
}

int WaveformDisplayComponent::findNearestWavetablePoint(const juce::Point<float>& position, const juce::Rectangle<float>& bounds)
{
    // TODO: Implement nearest point finding for wavetable editing
    return -1;
}

std::vector<float> WaveformDisplayComponent::getWavetableData() const
{
    return wavetableData;
}

void WaveformDisplayComponent::setWavetableData(const std::vector<float>& wavetable)
{
    wavetableData = wavetable;
    repaint();
}

juce::Point<float> WaveformDisplayComponent::sampleToPixel(int sampleIndex, float value, const juce::Rectangle<float>& bounds) const
{
    const float x = bounds.getX() + (static_cast<float>(sampleIndex) / static_cast<float>(maxSamples)) * bounds.getWidth();
    const float centerY = bounds.getCentreY();
    const float amplitude = bounds.getHeight() * 0.4f * settings.amplitudeScale;
    const float y = centerY - value * amplitude;
    return { x, y };
}

juce::Point<int> WaveformDisplayComponent::pixelToSample(const juce::Point<float>& pixel, const juce::Rectangle<float>& bounds) const
{
    const float normX = juce::jlimit(0.0f, 1.0f, (pixel.x - bounds.getX()) / bounds.getWidth());
    const int sampleIndex = static_cast<int>(normX * static_cast<float>(maxSamples - 1));
    const float centerY = bounds.getCentreY();
    const float amplitude = bounds.getHeight() * 0.4f * settings.amplitudeScale;
    const float value = juce::jlimit(-1.0f, 1.0f, (centerY - pixel.y) / amplitude);
    // Encode value as scaled integer for convenience (not used by tests)
    return { sampleIndex, static_cast<int>(value * 1000.0f) };
}
