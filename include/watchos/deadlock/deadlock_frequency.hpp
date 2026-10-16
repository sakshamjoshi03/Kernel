#ifndef WATCHOS_DEADLOCK_DEADLOCK_FREQUENCY_HPP
#define WATCHOS_DEADLOCK_DEADLOCK_FREQUENCY_HPP

#include "watchos/deadlock/resource_manager.hpp"
#include <vector>
#include <string>
#include <map>

namespace watchos {
namespace deadlock {

struct FrequencyResult {
    uint64_t randomSeed = 42;
    int32_t totalTrials = 1000;
    int32_t deadlockedTrials = 0;
    double empiricalFrequency = 0.0;
    std::map<core::ProcessId, int32_t> processInvolvementCount;
    std::string report;
};

class DeadlockFrequencyEstimator {
public:
    static FrequencyResult estimate(
        const ResourceManager& baseRm,
        const std::vector<core::ProcessId>& pids,
        int32_t trials = 1000,
        uint64_t seed = 42
    );
};

} // namespace deadlock
} // namespace watchos

#endif // WATCHOS_DEADLOCK_DEADLOCK_FREQUENCY_HPP
