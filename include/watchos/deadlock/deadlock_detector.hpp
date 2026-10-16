#ifndef WATCHOS_DEADLOCK_DEADLOCK_DETECTOR_HPP
#define WATCHOS_DEADLOCK_DEADLOCK_DETECTOR_HPP

#include "watchos/deadlock/resource_manager.hpp"
#include <vector>
#include <string>

namespace watchos {
namespace deadlock {

struct DeadlockDetectionResult {
    bool isDeadlocked = false;
    std::vector<core::ProcessId> deadlockedProcesses;
    std::string report;
};

class DeadlockDetector {
public:
    static DeadlockDetectionResult detect(const ResourceManager& rm, const std::vector<core::ProcessId>& pids);
};

} // namespace deadlock
} // namespace watchos

#endif // WATCHOS_DEADLOCK_DEADLOCK_DETECTOR_HPP
