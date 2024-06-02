#include "common/memory_stats.h"
#include "common/globals.h"

MemoryStats ramStats;
uint64_t memStatsLastUpdatedMillis = 0;

void updateMemoryStats()
{
    if (millis() - memStatsLastUpdatedMillis < ramStatsUpdateIntervalMillis)
        return;
#ifdef ESP32
    uint32_t freeHeap = esp_get_free_heap_size();
    ramStats.addSample(freeHeap);

#elif defined(ESP8266)
    uint32_t freeHeap = ESP.getFreeHeap();
    uint8_t heapFragmentation = ESP.getHeapFragmentation();
    uint32_t maxFreeBlockSize = ESP.getMaxFreeBlockSize();
    ramStats.addSample(freeHeap, heapFragmentation, maxFreeBlockSize);
#endif
}
