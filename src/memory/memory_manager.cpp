#include "watchos/memory/memory_manager.hpp"
#include <random>
#include <algorithm>

namespace watchos {
namespace memory {

MemoryManager::MemoryManager(int32_t totalFrames, int32_t pageSize)
    : totalFrames_(std::max(1, totalFrames)), pageSize_(std::max(1, pageSize)) {}

std::vector<int32_t> MemoryManager::generateReferenceString(const process::Process& proc, size_t length) const {
    if (!proc.getMemoryConfig().pageReferences.empty()) {
        return proc.getMemoryConfig().pageReferences;
    }

    int32_t demand = static_cast<int32_t>(proc.getMemoryConfig().demandUnits);
    if (demand <= 0) {
        demand = 32; // Default 32 units if unconfigured
    }

    int32_t numPages = std::max(1, (demand + pageSize_ - 1) / pageSize_);
    std::vector<int32_t> refs;
    refs.reserve(length);

    // Locality of reference: 80% of references target working set of ~30% pages
    std::mt19937 rng(1337 + proc.getPid());
    int32_t workingSetSize = std::max(1, (numPages * 3) / 10);

    for (size_t i = 0; i < length; ++i) {
        if (rng() % 100 < 80) {
            refs.push_back(rng() % workingSetSize);
        } else {
            refs.push_back(rng() % numPages);
        }
    }

    return refs;
}

std::vector<PageSimulationResult> MemoryManager::runComparativeAnalysis(
    const std::vector<process::Process>& processes,
    int32_t frameCount
) {
    std::vector<int32_t> combinedRefs;
    for (const auto& proc : processes) {
        auto refs = generateReferenceString(proc, 15);
        combinedRefs.insert(combinedRefs.end(), refs.begin(), refs.end());
    }

    if (combinedRefs.empty()) {
        // Fallback default reference sequence
        combinedRefs = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    }

    std::vector<PageSimulationResult> results;
    results.push_back(PageReplacementSimulator::simulate(PageReplacementPolicy::FIFO, combinedRefs, frameCount));
    results.push_back(PageReplacementSimulator::simulate(PageReplacementPolicy::LRU, combinedRefs, frameCount));
    results.push_back(PageReplacementSimulator::simulate(PageReplacementPolicy::OPTIMAL, combinedRefs, frameCount));

    return results;
}

ProcessPageTable MemoryManager::buildPageTable(const process::Process& proc) const {
    ProcessPageTable table;
    table.pid = proc.getPid();

    int32_t demand = static_cast<int32_t>(proc.getMemoryConfig().demandUnits);
    if (demand <= 0) demand = 16;
    table.totalVirtualPages = std::max(1, (demand + pageSize_ - 1) / pageSize_);

    // Map first few pages to frames
    for (int32_t p = 0; p < table.totalVirtualPages; ++p) {
        table.pageToFrame[p] = (p < totalFrames_) ? p : -1;
    }

    return table;
}

} // namespace memory
} // namespace watchos
