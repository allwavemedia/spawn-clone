//=========================================================================
//  SampleCache.cpp
//  SpawnClone – Advanced Synthesis Engine
//
//  Minimal LRU cache implementation (Phase-1).  Designed for real-time
//  safety: get() is lock-free, while insertion & eviction occur on the
//  non-RT loader thread.
//
//  NOTE:  Optimised chunk-level streaming will be added in a future story.
//=========================================================================
#include "SampleCache.h"

namespace spawnclone::audio
{

//-------------------------------------------------------------------------
//  Helpers
//-------------------------------------------------------------------------
static constexpr size_t toBytesMB(size_t mb) noexcept { return mb * 1024 * 1024; }

//-------------------------------------------------------------------------
//  Ctor / Capacity
//-------------------------------------------------------------------------
SampleCache::SampleCache(size_t capacityMB)
    : maxCapacity(toBytesMB(capacityMB))
{
}

void SampleCache::setCapacity(size_t capacityMB)
{
    std::lock_guard<std::mutex> lock(mutex);
    maxCapacity = toBytesMB(capacityMB);
    evictIfNecessary(0);  // trim immediately if needed
}

//-------------------------------------------------------------------------
//  Clear
//-------------------------------------------------------------------------
void SampleCache::clear()
{
    std::lock_guard<std::mutex> lock(mutex);
    lruList.clear();
    index.clear();
    currentSize = 0;
}

//-------------------------------------------------------------------------
//  get  (real-time safe)
//-------------------------------------------------------------------------
const juce::AudioBuffer<float>* SampleCache::get(uint64_t key) const noexcept
{
    // Fast path: atomic look-up using double-checked pattern
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = index.find(key);
        if (it != index.end())
        {
            // Move entry to front (MRU) – still under lock (non-RT)
            lruList.splice(lruList.begin(), lruList, it->second);
            stats.hits++;
            return it->second->buffer.get();
        }
    }
    stats.misses++;
    return nullptr;
}

//-------------------------------------------------------------------------
//  put (non-RT safe)
//-------------------------------------------------------------------------
bool SampleCache::put(uint64_t key, std::unique_ptr<juce::AudioBuffer<float>> buffer)
{
    if (!buffer)
        return false;

    const size_t bytes = static_cast<size_t>(buffer->getNumSamples() * buffer->getNumChannels() * sizeof(float));

    std::lock_guard<std::mutex> lock(mutex);

    if (bytes > maxCapacity)
        return false;  // too large

    // Replace existing
    if (auto it = index.find(key); it != index.end())
    {
        currentSize -= it->second->sizeBytes;
        lruList.erase(it->second);
        index.erase(it);
    }

    evictIfNecessary(bytes);

    lruList.push_front({key, std::move(buffer), bytes});
    index[key] = lruList.begin();
    currentSize += bytes;
    return true;
}

//-------------------------------------------------------------------------
//  Eviction (non-RT)
//-------------------------------------------------------------------------
void SampleCache::evictIfNecessary(size_t bytesNeeded)
{
    while (currentSize + bytesNeeded > maxCapacity && !lruList.empty())
    {
        auto& back = lruList.back();
        currentSize -= back.sizeBytes;
        index.erase(back.key);
        lruList.pop_back();
        stats.evictions++;
    }
}

} // namespace spawnclone::audio