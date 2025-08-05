//=========================================================================
//  AsyncSampleLoader.h
//  SpawnClone – Advanced Synthesis Engine
//
//  Background thread-pool that loads large sample files or blocks from
//  disk into SampleCache without blocking the real-time audio thread.
//
//  Strategy:
//    • Uses JUCE ThreadPool (default 2 workers, configurable)
//    • Call enqueueLoad() from message/pre-load thread
//    • Worker fetches file/block, creates AudioBuffer<float>, inserts
//      into SampleCache via put()
//
//  Created: 2025-08-05
//=========================================================================
#pragma once

#include "SampleCache.h"
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>

namespace spawnclone::audio
{

/**
 * \brief Background loader for SampleCache.
 *
 * FileKey encodes (sampleFilePathHash << 32 | layerIndex). For MVP we use
 * full-file loading; chunk support will be added later.
 */
class AsyncSampleLoader : private juce::ThreadPoolJob
{
public:
    /** Create loader bound to given cache and thread pool size (workers). */
    explicit AsyncSampleLoader(SampleCache& cacheRef, int workers = 2);

    ~AsyncSampleLoader() override;

    /** Queue a sample file for loading. Returns job id token. */
    juce::String enqueueLoad(uint64_t key,
                             const juce::File& fileToLoad,
                             int numChannelsExpected = 1);

    /** Cancel all pending jobs and block until finished. */
    void cancelAllJobs();

private:
    // juce::ThreadPoolJob impl
    JobStatus runJob() override;
    JobStatus runJobInternal(); // Internal method for job wrapper

    struct JobItem
    {
        uint64_t key;
        juce::File file;
        int expectedChannels;
    };

    SampleCache& cache;
    juce::ThreadPool pool;
    juce::Array<JobItem> pending;
    juce::CriticalSection pendingLock;
};

} // namespace spawnclone::audio