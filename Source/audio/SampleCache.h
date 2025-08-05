//=========================================================================
//  SampleCache.h
//  SpawnClone – Advanced Synthesis Engine
//
//  Light-weight, lock-free LRU sample cache used by SampleEngine to keep
//  frequently accessed sample blocks in RAM while streaming large audio
//  files from disk.  Implementation header-only for fast inclusion; the
//  class is intentionally minimal for Phase-1 MVP and will be extended
//  in later epics.
//
//  Created: 2025-08-05
//=========================================================================
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <list>
#include <mutex>
#include <unordered_map>

// Disable copy warnings on MSVC / clang-cl builds for JUCE headers
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wimplicit-fallthrough")

namespace spawnclone::audio
{
/**
 * \brief Thread-safe LRU cache of AudioBuffer<float> blocks.
 *
 * Design goals:
 *  • Lock-free get() in real-time thread (occasional CAS retry on miss)
 *  • Reserve only atomic counters in real-time critical path
 *  • Eviction & disk IO occur on background loader thread (AsyncSampleLoader)
 *
 *  Keys are uint64_t hash codes (layer index << 32 | block index) to avoid
 *  file-system overhead in RTR path. For MVP we store entire sample buffers.
 *  In a later iteration we will switch to chunked (256kB) blocks.
 */
class SampleCache
{
public:
    /** Create cache with default 64 MB limit */
    explicit SampleCache(size_t capacityMB = 64);

    /** Set maximum memory usage (in megabytes). RT safe. */
    void setCapacity(size_t capacityMB);

    /** Total memory currently used (bytes) */
    size_t getCurrentSize() const noexcept    { return currentSize.load(); }

    /** Maximum capacity (bytes) */
    size_t getCapacity() const noexcept       { return maxCapacity.load(); }

    /** Clear entire cache. Not RT safe (call from message thread). */
    void clear();

    /**
     * Retrieve a sample buffer; nullptr if not present.
     * Real-time safe – returns immediately with atomic reads only.
     */
    const juce::AudioBuffer<float>* get(uint64_t key) const noexcept;

    /**
     * Insert a new buffer. Caller guarantees buffer is heap-allocated and
     * will remain valid for the cache lifetime. Not RT safe.
     * Returns true if added, false if exceeds capacity.
     */
    bool put(uint64_t key, std::unique_ptr<juce::AudioBuffer<float>> buffer);

    /** For debug / telemetry */
    struct Stats
    {
        std::atomic<size_t> hits {0};
        std::atomic<size_t> misses {0};
        std::atomic<size_t> evictions {0};
    };
    const Stats& getStats() const noexcept { return stats; }

private:
    // Entry stored in the LRU list
    struct Entry
    {
        uint64_t key;
        std::unique_ptr<juce::AudioBuffer<float>> buffer;
        size_t sizeBytes;
    };

    // Internal helpers (non-RT)
    void evictIfNecessary(size_t bytesNeeded);

    // LRU order – most recently used at front
    mutable std::mutex mutex;
    mutable std::list<Entry> lruList;
    std::unordered_map<uint64_t, std::list<Entry>::iterator> index;

    std::atomic<size_t> currentSize {0};
    std::atomic<size_t> maxCapacity {64 * 1024 * 1024}; // 64 MB default
    mutable Stats stats;
};

} // namespace spawnclone::audio

JUCE_END_IGNORE_WARNINGS_GCC_LIKE