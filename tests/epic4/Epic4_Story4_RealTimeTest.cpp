/*
  ==============================================================================

    Epic4_Story4_RealTimeTest.cpp
    Created: 3 Aug 2025
    Author:  Epic 8 Story 8.4 Test Implementation

    Test suite for Epic 8 Story 8.4: Advanced Real-Time Processing
    Tests lock-free parameter updates, smooth interpolation, and performance optimization.

  ==============================================================================
*/

#include "../Source/audio/RealTimeProcessor.h"
#include "../Source/audio/LayerEffectsProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <cassert>
#include <iostream>
#include <chrono>
#include <thread>

//==============================================================================
class Epic4_Story4_RealTimeTest
{
public:
    Epic4_Story4_RealTimeTest()
    {
        std::cout << "\n=== Epic 8 Story 8.4: Advanced Real-Time Processing Test ===" << std::endl;
    }
    
    void runAllTests()
    {
        testRealTimeProcessorBasics();
        testLockFreeParameterUpdates();
        testParameterSmoothing();
        testPerformanceMonitoring();
        testLayerEffectsIntegration();
        testRealTimeConstraints();
        
        std::cout << "\n✅ All Epic 8 Story 8.4 tests passed!" << std::endl;
        std::cout << "Advanced real-time processing implementation complete." << std::endl;
    }

private:
    void testRealTimeProcessorBasics()
    {
        std::cout << "\n🔧 Testing RealTimeProcessor basics..." << std::endl;
        
        RealTimeProcessor processor;
        
        // Test preparation
        processor.prepareToPlay(44100.0, 512);
        assert(processor.isRealTimeSafe());
        
        // Test parameter setting
        processor.setParameterTarget(RealTimeProcessor::ParameterType::FilterCutoff, 0.5f);
        processor.setParameterTarget(RealTimeProcessor::ParameterType::ReverbMix, 0.3f);
        
        // Process some smoothing
        processor.processParameterSmoothing(512);
        
        // Check parameter values
        float filterValue = processor.getCurrentParameterValue(RealTimeProcessor::ParameterType::FilterCutoff);
        float reverbValue = processor.getCurrentParameterValue(RealTimeProcessor::ParameterType::ReverbMix);
        
        assert(filterValue >= 0.0f && filterValue <= 1.0f);
        assert(reverbValue >= 0.0f && reverbValue <= 1.0f);
        
        processor.releaseResources();
        
        std::cout << "✓ RealTimeProcessor basics working correctly" << std::endl;
    }
    
    void testLockFreeParameterUpdates()
    {
        std::cout << "\n🔒 Testing lock-free parameter updates..." << std::endl;
        
        RealTimeProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        // Test concurrent parameter updates (simulating UI thread and audio thread)
        std::atomic<bool> testRunning{true};
        std::atomic<int> updateCount{0};
        
        // Simulate UI thread updating parameters
        std::thread uiThread([&]() {
            while (testRunning.load())
            {
                float value = static_cast<float>(rand()) / RAND_MAX;
                processor.setParameterTarget(RealTimeProcessor::ParameterType::FilterCutoff, value);
                updateCount.fetch_add(1);
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        });
        
        // Simulate audio thread processing
        std::thread audioThread([&]() {
            for (int i = 0; i < 100; ++i)
            {
                processor.processParameterSmoothing(512);
                std::this_thread::sleep_for(std::chrono::microseconds(250));
            }
            testRunning.store(false);
        });
        
        uiThread.join();
        audioThread.join();
        
        assert(updateCount.load() > 0);
        assert(processor.isRealTimeSafe());
        
        processor.releaseResources();
        
        std::cout << "✓ Lock-free updates processed " << updateCount.load() << " parameter changes" << std::endl;
    }
    
    void testParameterSmoothing()
    {
        std::cout << "\n📈 Testing parameter smoothing..." << std::endl;
        
        RealTimeProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        // Set a target value and track smoothing
        processor.setParameterTarget(RealTimeProcessor::ParameterType::Volume, 1.0f, 0.1f); // 100ms smoothing
        
        float previousValue = 0.0f;
        float currentValue = 0.0f;
        bool smoothingObserved = false;
        
        // Process several blocks and observe smoothing
        for (int block = 0; block < 20; ++block)
        {
            processor.processParameterSmoothing(512);
            currentValue = processor.getCurrentParameterValue(RealTimeProcessor::ParameterType::Volume);
            
            // Check for smooth progression
            if (currentValue > previousValue && currentValue < 1.0f)
            {
                smoothingObserved = true;
            }
            
            previousValue = currentValue;
        }
        
        assert(smoothingObserved);
        assert(currentValue >= 0.9f); // Should be close to target by now
        
        processor.releaseResources();
        
        std::cout << "✓ Parameter smoothing working correctly (final value: " 
                  << currentValue << ")" << std::endl;
    }
    
    void testPerformanceMonitoring()
    {
        std::cout << "\n📊 Testing performance monitoring..." << std::endl;
        
        RealTimeProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        // Simulate some processing load
        for (int i = 0; i < 10; ++i)
        {
            processor.startBlockTiming();
            
            // Simulate processing work
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            
            processor.endBlockTiming();
            processor.processParameterSmoothing(512);
        }
        
        auto metrics = processor.getPerformanceMetrics();
        
        assert(metrics.averageBlockTime > 0.0);
        assert(metrics.parameterUpdates >= 0);
        
        // Test CPU usage reporting
        processor.updateCPUUsage(0.5f);
        auto updatedMetrics = processor.getPerformanceMetrics();
        assert(updatedMetrics.cpuUsage == 0.5f);
        
        processor.releaseResources();
        
        std::cout << "✓ Performance monitoring active (avg block time: " 
                  << metrics.averageBlockTime << "ms)" << std::endl;
    }
    
    void testLayerEffectsIntegration()
    {
        std::cout << "\n🎛️ Testing LayerEffectsProcessor integration..." << std::endl;
        
        LayerEffectsProcessor processor;
        processor.prepareToPlay(44100.0, 512, 2);
        
        // Test smooth parameter setting
        processor.setParameterSmooth(LayerEffectsProcessor::LayerType::Melody, 
                                   RealTimeProcessor::ParameterType::FilterCutoff, 
                                   0.7f, 0.05f);
        
        processor.setParameterSmooth(LayerEffectsProcessor::LayerType::Chords, 
                                   RealTimeProcessor::ParameterType::ReverbMix, 
                                   0.4f);
        
        // Create test audio buffer
        juce::AudioBuffer<float> testBuffer(2, 512);
        testBuffer.clear();
        
        // Add some test signal
        for (int channel = 0; channel < 2; ++channel)
        {
            auto* channelData = testBuffer.getWritePointer(channel);
            for (int sample = 0; sample < 512; ++sample)
            {
                channelData[sample] = std::sin(2.0 * M_PI * 440.0 * sample / 44100.0) * 0.1f;
            }
        }
        
        // Process the buffer
        processor.processLayer(LayerEffectsProcessor::LayerType::Melody, testBuffer);
        
        // Check performance metrics
        auto metrics = processor.getPerformanceMetrics();
        assert(processor.isRealTimeSafe());
        
        processor.releaseResources();
        
        std::cout << "✓ LayerEffectsProcessor integration working correctly" << std::endl;
    }
    
    void testRealTimeConstraints()
    {
        std::cout << "\n⏱️ Testing real-time constraints..." << std::endl;
        
        RealTimeProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        // Test normal operation
        processor.startBlockTiming();
        std::this_thread::sleep_for(std::chrono::microseconds(1000)); // 1ms - should be fine
        processor.endBlockTiming();
        
        assert(processor.isRealTimeSafe());
        
        // Test constraint validation
        processor.validateRealTimeConstraints();
        
        // Test memory allocation reporting
        processor.reportMemoryAllocation(1024);
        assert(processor.getTotalAllocations() == 1024);
        
        processor.releaseResources();
        
        std::cout << "✓ Real-time constraints validation working" << std::endl;
    }
};

//==============================================================================
int main()
{
    try
    {
        Epic4_Story4_RealTimeTest test;
        test.runAllTests();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
