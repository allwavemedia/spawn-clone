/*
  ==============================================================================

    ThreadManager.h
    Created: 28 Jul 2025
    Author:  Developer Agent

    Manages the lifecycle of background threads.

  ==============================================================================
*/

#pragma once

#include <juce_events/juce_events.h>
#include <functional>

//==============================================================================
/**
 * Manages the lifecycle of background threads, specifically the AI worker thread.
 * Provides a simple interface for submitting tasks to be run off the main threads.
 */
class ThreadManager : public juce::Thread
{
public:
    //==============================================================================
    ThreadManager();
    ~ThreadManager() override;

    //==============================================================================
    /** Submits a task to be executed on the background thread. */
    void submitTask(std::function<void()> task);

private:
    //==============================================================================
    void run() override;

    //==============================================================================
    std::function<void()> taskToRun;
    juce::WaitableEvent taskReady;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThreadManager)
};
