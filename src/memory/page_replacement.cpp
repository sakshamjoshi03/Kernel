#include "watchos/memory/page_replacement.hpp"
#include <algorithm>
#include <deque>
#include <map>
#include <sstream>
#include <iomanip>

namespace watchos {
namespace memory {

PageSimulationResult PageReplacementSimulator::simulate(
    PageReplacementPolicy policy,
    const std::vector<int32_t>& referenceString,
    int32_t frameCount
) {
    PageSimulationResult res;
    res.policy = policy;
    res.algorithmName = policyToString(policy);
    res.frameCount = std::max(1, frameCount);
    res.totalReferences = referenceString.size();

    if (referenceString.empty()) {
        return res;
    }

    std::vector<int32_t> frames; // current frames in memory
    std::deque<int32_t> fifoOrder; // for FIFO
    std::map<int32_t, size_t> lastUsedTime; // for LRU

    for (size_t refIdx = 0; refIdx < referenceString.size(); ++refIdx) {
        int32_t page = referenceString[refIdx];
        auto it = std::find(frames.begin(), frames.end(), page);
        bool isHit = (it != frames.end());

        if (isHit) {
            res.pageHits++;
            res.faultFlags.push_back(false);
            if (policy == PageReplacementPolicy::LRU) {
                lastUsedTime[page] = refIdx;
            }
        } else {
            res.pageFaults++;
            res.faultFlags.push_back(true);

            if (static_cast<int32_t>(frames.size()) < res.frameCount) {
                // Free frame available
                frames.push_back(page);
                if (policy == PageReplacementPolicy::FIFO) {
                    fifoOrder.push_back(page);
                } else if (policy == PageReplacementPolicy::LRU) {
                    lastUsedTime[page] = refIdx;
                }
            } else {
                // Must replace a victim page
                res.replacements++;
                int victimPage = -1;

                if (policy == PageReplacementPolicy::FIFO) {
                    victimPage = fifoOrder.front();
                    fifoOrder.pop_front();
                    fifoOrder.push_back(page);
                } else if (policy == PageReplacementPolicy::LRU) {
                    size_t oldestTime = refIdx + 1;
                    for (int32_t f : frames) {
                        if (lastUsedTime[f] < oldestTime) {
                            oldestTime = lastUsedTime[f];
                            victimPage = f;
                        }
                    }
                    lastUsedTime[page] = refIdx;
                } else if (policy == PageReplacementPolicy::OPTIMAL) {
                    // Find page in frames that will not be used for longest time in future
                    size_t furthestUse = 0;
                    for (int32_t f : frames) {
                        size_t nextUse = referenceString.size() + 1; // not used again
                        for (size_t k = refIdx + 1; k < referenceString.size(); ++k) {
                            if (referenceString[k] == f) {
                                nextUse = k;
                                break;
                            }
                        }
                        if (nextUse > furthestUse) {
                            furthestUse = nextUse;
                            victimPage = f;
                        }
                    }
                }

                // Replace victim in frames
                auto victimIt = std::find(frames.begin(), frames.end(), victimPage);
                if (victimIt != frames.end()) {
                    *victimIt = page;
                } else if (!frames.empty()) {
                    frames[0] = page;
                }
            }
        }

        res.frameSnapshots.push_back(frames);
    }

    if (res.totalReferences > 0) {
        res.faultRatio = static_cast<double>(res.pageFaults) / res.totalReferences;
        // PRD Section 6.9: Thrashing indicator when page fault ratio exceeds 70% with heavy replacement
        res.thrashingDetected = (res.faultRatio > 0.70 && res.replacements > (res.totalReferences / 2));
    }

    return res;
}

std::string PageReplacementSimulator::formatResultTable(const PageSimulationResult& result) {
    std::ostringstream oss;
    oss << "Page Replacement Simulation [" << result.algorithmName << "] (" << result.frameCount << " Physical Frames):\n";
    oss << "+------------------------+--------------------+\n";
    oss << "| Total References       | " << std::setw(18) << result.totalReferences << " |\n";
    oss << "| Page Faults            | " << std::setw(18) << result.pageFaults << " |\n";
    oss << "| Page Hits              | " << std::setw(18) << result.pageHits << " |\n";
    oss << "| Fault Ratio            | " << std::setw(17) << std::fixed << std::setprecision(2) << (result.faultRatio * 100.0) << "% |\n";
    oss << "| Total Replacements     | " << std::setw(18) << result.replacements << " |\n";
    oss << "| Thrashing Risk         | " << std::setw(18) << (result.thrashingDetected ? "WARNING: HIGH" : "NORMAL") << " |\n";
    oss << "+------------------------+--------------------+\n";
    return oss.str();
}

} // namespace memory
} // namespace watchos
