//=========================================================================
//  AsyncSampleLoader.cpp
//  SpawnClone – Advanced Synthesis Engine
//
//  NOTE: MVP implementation for background sample loading.  Actual streaming
//  chunk logic will be implemented in follow-up tasks.
//=========================================================================
#include "AsyncSampleLoader.h"

namespace spawnclone::audio
{

//-------------------------------------------------------------------------
//  Constructor / Destructor
//-------------------------------------------------------------------------
AsyncSampleLoader::AsyncSampleLoader(SampleCache& cacheRef, int workers)
    : ThreadPoolJob("AsyncSampleLoaderRoot"),
      cache(cacheRef),
      pool(workers)
{
}

AsyncSampleLoader::~AsyncSampleLoader()
{
    cancelAllJobs();
}

//-------------------------------------------------------------------------
//  Public API
//-------------------------------------------------------------------------
juce::String AsyncSampleLoader::enqueueLoad(uint64_t key,
                                           const juce::File& fileToLoad,
                                           int numChannelsExpected)
{
    JobItem item { key, fileToLoad, numChannelsExpected };
    {
        const juce::ScopedLock lock(pendingLock);
        pending.add(item);
    }

    // Submit a lightweight job object - create a simple wrapper instead of copying
    class LoadJob : public juce::ThreadPoolJob
    {
    public:
        LoadJob(AsyncSampleLoader& loader) : juce::ThreadPoolJob("SampleLoad"), loader_(loader) {}
        
        JobStatus runJob() override
        {
            return loader_.runJobInternal();
        }
        
    private:
        AsyncSampleLoader& loader_;
    };
    
    auto* job = new LoadJob(*this);
    pool.addJob(job, false);

    return fileToLoad.getFileName();
}

void AsyncSampleLoader::cancelAllJobs()
{
    pool.removeAllJobs(true, 10000); // wait up to 10s
    const juce::ScopedLock lock(pendingLock);
    pending.clear();
}

//-------------------------------------------------------------------------
//  Worker (ThreadPoolJob)
//-------------------------------------------------------------------------
juce::ThreadPoolJob::JobStatus AsyncSampleLoader::runJobInternal()
{
    return runJob();
}

juce::ThreadPoolJob::JobStatus AsyncSampleLoader::runJob()
{
    JobItem current;
    {
        const juce::ScopedLock lock(pendingLock);
        if (pending.isEmpty())
            return juce::ThreadPoolJob::jobHasFinished;

        current = pending.getReference(0);
        pending.remove(0);
    }

    // Already cached?
    if (cache.get(current.key) != nullptr)
        return juce::ThreadPoolJob::jobHasFinished;

    // Load audio file
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(current.file));
    if (!reader)
        return juce::ThreadPoolJob::jobHasFinished;

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(
        static_cast<int>(reader->numChannels),
        static_cast<int>(reader->lengthInSamples));

    reader->read(buffer.get(), 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

    // Insert into cache (may fail if memory cap too low)
    cache.put(current.key, std::move(buffer));

    return juce::ThreadPoolJob::jobHasFinished;
}

} // namespace spawnclone::audio