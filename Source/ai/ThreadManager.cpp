/*
  ==============================================================================

    ThreadManager.cpp
    Created: 28 Jul 2025
    Author:  Developer Agent

    Implementation of the thread manager.

  ==============================================================================
*/

#include "ThreadManager.h"

//==============================================================================
ThreadManager::ThreadManager() : Thread("AI Worker Thread")
{
    startThread();
}

ThreadManager::~ThreadManager()
{
    stopThread(5000); // Wait up to 5 seconds for the thread to stop
}

//==============================================================================
void ThreadManager::submitTask(std::function<void()> task)
{
    taskToRun = std::move(task);
    taskReady.signal();
}

//==============================================================================
void ThreadManager::run()
{
    while (!threadShouldExit())
    {
        if (taskReady.wait(-1)) // Wait indefinitely for a task
        {
            if (taskToRun)
            {
                taskToRun();
                taskToRun = nullptr; // Reset the task
            }
        }
    }
}
