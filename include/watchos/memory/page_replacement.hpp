#ifndef WATCHOS_MEMORY_PAGE_REPLACEMENT_HPP
#define WATCHOS_MEMORY_PAGE_REPLACEMENT_HPP

#include <vector>
#include <string>
#include <cstdint>

namespace watchos {
namespace memory {

enum class PageReplacementPolicy {
    FIFO,
    LRU,
    OPTIMAL
};

inline std::string policyToString(PageReplacementPolicy policy) {
    switch (policy) {
        case PageReplacementPolicy::FIFO: return "FIFO";
        case PageReplacementPolicy::LRU: return "LRU";
        case PageReplacementPolicy::OPTIMAL: return "Optimal";
        default: return "Unknown";
    }
}

struct PageSimulationResult {
    PageReplacementPolicy policy;
    std::string algorithmName;
    int32_t frameCount = 3;
    size_t totalReferences = 0;
    size_t pageFaults = 0;
    size_t pageHits = 0;
    size_t replacements = 0;
    double faultRatio = 0.0;
    bool thrashingDetected = false;
    std::vector<std::vector<int32_t>> frameSnapshots; // frames at each reference
    std::vector<bool> faultFlags;                    // true if reference caused fault
};

class PageReplacementSimulator {
public:
    static PageSimulationResult simulate(
        PageReplacementPolicy policy,
        const std::vector<int32_t>& referenceString,
        int32_t frameCount
    );

    static std::string formatResultTable(const PageSimulationResult& result);
};

} // namespace memory
} // namespace watchos

#endif // WATCHOS_MEMORY_PAGE_REPLACEMENT_HPP
