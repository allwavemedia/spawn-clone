//==============================================================================
/*
    SpawnClone Live Performance Demo
    
    Demonstrates the integration of real-time AI-driven live performance
    with the optimized ONNX daemon and advanced synthesis engine.
    
    This demo shows:
    - Real-time pattern generation using the fast ONNX daemon
    - AI-driven synthesis parameter modulation
    - Pattern evolution and morphing
    - Live performance triggers and automation
*/
//==============================================================================

#include <JuceHeader.h>
#include "Source/ai/LivePerformanceEngine.h"
#include "Source/processor/LivePerformanceIntegration.h"
#include "Source/ai/ONNXDaemonClient.h"
#include "Source/audio/AudioPreviewEngine.h"
#include "Source/audio/AdvancedSynthesisEngine.h"
#include "Source/GenerationParameters.h"

class LivePerformanceDemo : public juce::Component,
                           public juce::Timer,
                           public spawnclone::processor::LivePerformanceIntegration::Listener
{
public:
    LivePerformanceDemo()
    {
        // Set up the demo window
        setSize(800, 600);
        setWantsKeyboardFocus(true);
        
        // Initialize demo
        if (initializeDemo())
        {
            std::cout << "✅ Live Performance Demo initialized successfully!\n";
            std::cout << "🎹 Press SPACE to trigger pattern generation\n";
            std::cout << "🎵 Press 'E' to start/stop pattern evolution\n";
            std::cout << "🎛️ Press 'M' to toggle AI parameter modulation\n";
            std::cout << "🔄 Press 'V' to generate pattern variation\n";
            std::cout << "📊 Press 'S' to show performance statistics\n";
            
            startTimer(1000); // Update UI every second
        }
        else
        {
            std::cout << "❌ Failed to initialize Live Performance Demo\n";
        }
    }
    
    ~LivePerformanceDemo() override
    {
        stopTimer();
        
        if (livePerformance)
        {
            livePerformance->removeListener(this);
            livePerformance->setLivePerformanceEnabled(false);
        }
        
        if (audioPreviewEngine)
            audioPreviewEngine->releaseResources();
    }

private:
    // Core components
    std::unique_ptr<AudioPreviewEngine> audioPreviewEngine;
    std::shared_ptr<spawnclone::ai::ONNXDaemonClient> onnxClient;
    std::unique_ptr<spawnclone::processor::LivePerformanceIntegration> livePerformance;
    
    // Demo state
    bool isEvolutionActive = false;
    bool isModulationActive = false;
    GenerationParameters demoParams;
    
    //==============================================================================
    bool initializeDemo()
    {
        std::cout << "🚀 Initializing Live Performance Demo...\n";
        
        // 1. Initialize Audio Preview Engine
        std::cout << "🔊 Initializing Audio Preview Engine...\n";
        audioPreviewEngine = std::make_unique<AudioPreviewEngine>();
        audioPreviewEngine->prepareToPlay(44100.0, 512, 2);
        
        // Enable advanced synthesis
        audioPreviewEngine->setAdvancedSynthesisEnabled(true);
        std::cout << "   ✅ Advanced synthesis enabled\n";
        
        // 2. Initialize ONNX Daemon Client
        std::cout << "🤖 Initializing ONNX Daemon Client...\n";
        onnxClient = std::make_shared<spawnclone::ai::ONNXDaemonClient>();
        
        // Start the daemon (this will use your production-ready persistent daemon)
        if (!onnxClient->startDaemon())
        {
            std::cout << "   ❌ Failed to start ONNX daemon\n";
            return false;
        }
        std::cout << "   ✅ ONNX daemon started successfully\n";
        
        // Load a model (you'll need to have your MIDI model available)
        auto modelPath = juce::File::getCurrentWorkingDirectory().getChildFile("midi-model.onnx");
        if (modelPath.exists())
        {
            auto result = onnxClient->loadModel(modelPath.getFullPathName());
            if (result.success)
            {
                std::cout << "   ✅ MIDI model loaded successfully\n";
            }
            else
            {
                std::cout << "   ⚠️ Model load failed: " << result.error << "\n";
                std::cout << "   🔄 Continuing with demo (will use dummy patterns)\n";
            }
        }
        else
        {
            std::cout << "   ⚠️ Model file not found at: " << modelPath.getFullPathName() << "\n";
            std::cout << "   🔄 Continuing with demo (will use dummy patterns)\n";
        }
        
        // 3. Initialize Live Performance Integration
        std::cout << "🎭 Initializing Live Performance Integration...\n";
        livePerformance = std::make_unique<spawnclone::processor::LivePerformanceIntegration>();
        
        if (!livePerformance->initialize(audioPreviewEngine.get(), onnxClient))
        {
            std::cout << "   ❌ Failed to initialize live performance integration\n";
            return false;
        }
        
        livePerformance->addListener(this);
        std::cout << "   ✅ Live performance integration ready\n";
        
        // 4. Configure demo parameters
        setupDemoParameters();
        
        // 5. Configure live performance settings
        configureLivePerformance();
        
        std::cout << "🎉 Live Performance Demo ready!\n";
        return true;
    }
    
    void setupDemoParameters()
    {
        // Set up some nice demo parameters
        demoParams.musicalKey = "C";
        demoParams.scale = "Major";
        demoParams.tempo = 120.0;
        demoParams.complexity = 0.6f;
        demoParams.generationType = GenerationParameters::GenerationType::Melody;
        
        livePerformance->setCurrentGenerationParameters(demoParams);
        
        std::cout << "🎼 Demo parameters configured:\n";
        std::cout << "   Key: " << demoParams.musicalKey << " " << demoParams.scale << "\n";
        std::cout << "   Tempo: " << demoParams.tempo << " BPM\n";
        std::cout << "   Complexity: " << demoParams.complexity << "\n";
    }
    
    void configureLivePerformance()
    {
        // Enable live performance
        livePerformance->setLivePerformanceEnabled(true);
        
        // Start with pattern generation mode
        livePerformance->setPerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode::PatternGeneration);
        
        // Configure AI modulation targets
        spawnclone::ai::LivePerformanceEngine::ModulationTargets targets;
        targets.filterCutoff = true;
        targets.filterResonance = true;
        targets.wavetablePosition = true;
        targets.lfoDepth = true;
        targets.effectsParams = true;
        livePerformance->setModulationTargets(targets);
        
        // Configure evolution settings
        spawnclone::ai::LivePerformanceEngine::EvolutionSettings evolution;
        evolution.mutationRate = 0.1f;
        evolution.rhythmVariation = 0.2f;
        evolution.harmonicShift = 0.15f;
        evolution.timbreEvolution = 0.25f;
        evolution.maintainKey = true;
        evolution.maintainStructure = true;
        livePerformance->setEvolutionSettings(evolution);
        
        std::cout << "⚙️ Live performance configured with AI modulation and evolution\n";
    }
    
    //==============================================================================
    // UI and Interaction
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
        
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        
        // Title
        g.drawText("🎭 SpawnClone Live Performance Demo", 20, 20, getWidth() - 40, 30,
                   juce::Justification::centred);
        
        // Status
        auto y = 70;
        g.setFont(14.0f);
        
        g.drawText("Status: " + (livePerformance->isLivePerformanceEnabled() ? 
                   "🟢 LIVE ENABLED" : "🔴 DISABLED"), 20, y, 300, 20,
                   juce::Justification::left);
        y += 25;
        
        g.drawText("Evolution: " + juce::String(isEvolutionActive ? "🔄 ACTIVE" : "⏸️ STOPPED"),
                   20, y, 300, 20, juce::Justification::left);
        y += 25;
        
        g.drawText("AI Modulation: " + juce::String(isModulationActive ? "🎛️ ACTIVE" : "🚫 OFF"),
                   20, y, 300, 20, juce::Justification::left);
        y += 25;
        
        // Performance metrics
        y += 20;
        g.setFont(12.0f);
        g.drawText("Performance Metrics:", 20, y, 300, 20, juce::Justification::left);
        y += 20;
        
        auto metrics = livePerformance->getPerformanceMetrics();
        auto lines = juce::StringArray::fromLines(metrics);
        
        for (const auto& line : lines)
        {
            g.drawText(line, 30, y, getWidth() - 60, 15, juce::Justification::left);
            y += 18;
        }
        
        // Controls
        y += 30;
        g.setFont(14.0f);
        g.drawText("Controls:", 20, y, 200, 20, juce::Justification::left);
        y += 25;
        
        g.setFont(12.0f);
        g.drawText("SPACE - Trigger Pattern Generation", 30, y, 400, 15, juce::Justification::left);
        y += 18;
        g.drawText("E - Start/Stop Pattern Evolution", 30, y, 400, 15, juce::Justification::left);
        y += 18;
        g.drawText("M - Toggle AI Parameter Modulation", 30, y, 400, 15, juce::Justification::left);
        y += 18;
        g.drawText("V - Generate Pattern Variation", 30, y, 400, 15, juce::Justification::left);
        y += 18;
        g.drawText("S - Show Performance Statistics", 30, y, 400, 15, juce::Justification::left);
        y += 18;
        g.drawText("Q - Quit Demo", 30, y, 400, 15, juce::Justification::left);
    }
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::spaceKey)
        {
            std::cout << "🎵 Triggering pattern generation...\n";
            livePerformance->triggerPatternGeneration();
            return true;
        }
        else if (key.getKeyCode() == 'E' || key.getKeyCode() == 'e')
        {
            if (isEvolutionActive)
            {
                std::cout << "⏸️ Stopping pattern evolution\n";
                livePerformance->stopPatternEvolution();
                isEvolutionActive = false;
            }
            else
            {
                std::cout << "🔄 Starting pattern evolution\n";
                livePerformance->startPatternEvolution(0.1f);
                isEvolutionActive = true;
            }
            repaint();
            return true;
        }
        else if (key.getKeyCode() == 'M' || key.getKeyCode() == 'm')
        {
            isModulationActive = !isModulationActive;
            std::cout << (isModulationActive ? "🎛️ Enabling" : "🚫 Disabling") << " AI parameter modulation\n";
            
            livePerformance->configureAIModulation(isModulationActive, 0.6f);
            
            if (isModulationActive)
            {
                livePerformance->setPerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode::FullLive);
            }
            else
            {
                livePerformance->setPerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode::PatternGeneration);
            }
            
            repaint();
            return true;
        }
        else if (key.getKeyCode() == 'V' || key.getKeyCode() == 'v')
        {
            std::cout << "🔀 Generating pattern variation...\n";
            livePerformance->triggerPatternVariation(0.4f);
            return true;
        }
        else if (key.getKeyCode() == 'S' || key.getKeyCode() == 's')
        {
            std::cout << "\n📊 Performance Statistics:\n";
            std::cout << livePerformance->getPerformanceMetrics();
            std::cout << "\n";
            return true;
        }
        else if (key.getKeyCode() == 'Q' || key.getKeyCode() == 'q')
        {
            std::cout << "👋 Exiting Live Performance Demo\n";
            juce::JUCEApplication::quit();
            return true;
        }
        
        return false;
    }
    
    //==============================================================================
    // LivePerformanceIntegration::Listener Implementation
    
    void livePatternGenerated(std::shared_ptr<MIDIPattern> pattern) override
    {
        std::cout << "🎼 New pattern generated! Notes: " << (pattern ? pattern->notes.size() : 0) << "\n";
        repaint();
    }
    
    void liveSynthesisParametersChanged(const juce::StringPairArray& changes) override
    {
        std::cout << "🎛️ AI modulated " << changes.size() << " synthesis parameters\n";
        
        // Show parameter changes
        for (int i = 0; i < changes.size(); ++i)
        {
            std::cout << "   " << changes.getAllKeys()[i] << " = " << changes.getAllValues()[i] << "\n";
        }
    }
    
    void livePerformanceStateChanged() override
    {
        repaint();
    }
    
    void livePerformanceError(const juce::String& error) override
    {
        std::cout << "❌ Live Performance Error: " << error << "\n";
    }
    
    //==============================================================================
    // Timer callback for UI updates
    
    void timerCallback() override
    {
        repaint();
    }
};

//==============================================================================
class LivePerformanceDemoApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Live Performance Demo"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    
    void initialise(const juce::String& commandLine) override
    {
        mainWindow = std::make_unique<juce::DocumentWindow>(
            "SpawnClone Live Performance Demo",
            juce::Colours::darkgrey,
            juce::DocumentWindow::allButtons);
        
        mainWindow->setContentOwned(new LivePerformanceDemo(), true);
        mainWindow->centreWithSize(800, 600);
        mainWindow->setVisible(true);
    }
    
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
private:
    std::unique_ptr<juce::DocumentWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(LivePerformanceDemoApplication)
