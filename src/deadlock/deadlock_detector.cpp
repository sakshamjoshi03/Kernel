#include "watchos/deadlock/deadlock_detector.hpp"
#include <map>
#include <sstream>

namespace watchos {
namespace deadlock {

DeadlockDetectionResult DeadlockDetector::detect(const ResourceManager& rm, const std::vector<core::ProcessId>& pids) {
    DeadlockDetectionResult res;

    // Collect available instances Work
    std::map<std::string, int32_t> work;
    std::vector<std::string> rNames;
    for (const auto& rPair : rm.getResourceTypes()) {
        rNames.push_back(rPair.first);
        work[rPair.first] = rPair.second.availableInstances;
    }

    std::map<core::ProcessId, bool> finish;
    for (core::ProcessId pid : pids) {
        // If allocation is 0 for all resources, finish = true, else false
        bool hasAlloc = false;
        for (const auto& r : rNames) {
            if (rm.getAllocation(pid, r) > 0) {
                hasAlloc = true;
                break;
            }
        }
        finish[pid] = !hasAlloc;
    }

    bool progress = true;
    while (progress) {
        progress = false;
        for (core::ProcessId pid : pids) {
            if (finish[pid]) continue;

            bool canRun = true;
            for (const auto& r : rNames) {
                if (rm.getRequest(pid, r) > work[r]) {
                    canRun = false;
                    break;
                }
            }

            if (canRun) {
                for (const auto& r : rNames) {
                    work[r] += rm.getAllocation(pid, r);
                }
                finish[pid] = true;
                progress = true;
            }
        }
    }

    for (core::ProcessId pid : pids) {
        if (!finish[pid]) {
            res.isDeadlocked = true;
            res.deadlockedProcesses.push_back(pid);
        }
    }

    std::ostringstream oss;
    if (res.isDeadlocked) {
        oss << "[DEADLOCK DETECTED] The following processes are deadlocked: ";
        for (size_t i = 0; i < res.deadlockedProcesses.size(); ++i) {
            oss << "PID " << res.deadlockedProcesses[i] << (i + 1 < res.deadlockedProcesses.size() ? ", " : ".\n");
        }
    } else {
        oss << "[NO DEADLOCK] System is currently free of deadlocks. All processes can acquire needed resources.\n";
    }
    res.report = oss.str();

    return res;
}

} // namespace deadlock
} // namespace watchos
