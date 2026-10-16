#ifndef WATCHOS_MEMORY_MEMORY_MANAGER_HPP
#define WATCHOS_MEMORY_MEMORY_MANAGER_HPP

#include "watchos/process/process.hpp"
#include "watchos/memory/page_replacement.hpp"
#include <vector>
#include <string>
#include <map>

namespace watchos {
namespace memory {

struct ProcessPageTable {
    core::ProcessId pid = 0;
    int32_t totalVirtualPages = 0;
    std::map<int32_t, int32_t> pageToFrame; // page# -> frame# or -1 if unmapped
};

class MemoryManager {
public:
    explicit MemoryManager(int32_t totalFrames = 16, int32_t pageSize = 4);

    int32_t getTotalFrames() const { return totalFrames_; }
    int32_t getPageSize() const { return pageSize_; }

    // Generate explicit reference string for a process based on memory demand and locality
    std::vector<int32_t> generateReferenceString(const process::Process& proc, size_t length = 20) const;

    // Run full comparative memory analysis across FIFO, LRU, and Optimal
    std::vector<PageSimulationResult> runComparativeAnalysis(
        const std::vector<process::Process>& processes,
        int32_t frameCount = 3
    );

    ProcessPageTable buildPageTable(const process::Process& proc) const;

private:
    int32_t totalFrames_ = 16;
    int32_t pageSize_ = 4;
};

} // namespace memory
} // namespace watchos

#endif // WATCHOS_MEMORY_MEMORY_MANAGER_HPP
