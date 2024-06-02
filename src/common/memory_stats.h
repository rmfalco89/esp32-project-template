#ifndef MEMORY_STATS_H
#define MEMORY_STATS_H

#include "Arduino.h"

#include <vector>
#include <numeric> // For std::accumulate

struct MemoryStats
{
    std::vector<uint32_t> usageSamples;
    size_t currentSampleIndex = 0;
    bool isBufferFull = false;
    const size_t sampleSize = 60 * 24; // 24h at 60 measurements /h
    uint8_t heapFragmentation = 0;
    uint32_t maxFreeBlockSize = 0;

    MemoryStats()
    {
        usageSamples.resize(sampleSize, 0);
    }

    void addSample(uint32_t sample, uint8_t heapFragmentation_, uint32_t maxFreeBlockSize_)
    {
        usageSamples[currentSampleIndex] = sample;
        currentSampleIndex = (currentSampleIndex + 1) % sampleSize;
        if (currentSampleIndex == 0)
            isBufferFull = true;
        heapFragmentation = heapFragmentation_;
        maxFreeBlockSize = maxFreeBlockSize_;
    }

    void addSample(uint32_t sample)
    {
        addSample(sample, 0, 0);
    }

    uint32_t getMin() const
    {
        auto begin = usageSamples.begin();
        auto end = isBufferFull ? usageSamples.end() : begin + currentSampleIndex;
        return *std::min_element(begin, end);
    }

    uint32_t getMax() const
    {
        auto begin = usageSamples.begin();
        auto end = isBufferFull ? usageSamples.end() : begin + currentSampleIndex;
        return *std::max_element(begin, end);
    }

    double getAverage() const
    {
        auto begin = usageSamples.begin();
        auto end = isBufferFull ? usageSamples.end() : begin + currentSampleIndex;
        double sum = std::accumulate(begin, end, 0.0);
        size_t count = isBufferFull ? sampleSize : currentSampleIndex;
        return sum / count;
    }
};

void updateMemoryStats();

#endif