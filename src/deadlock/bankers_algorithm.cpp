#include "watchos/deadlock/bankers_algorithm.hpp"
#include <sstream>
#include <iomanip>
#include <set>

namespace watchos {
namespace deadlock {

BankersResult BankersAlgorithm::evaluateSafeState(const ResourceManager& rm, const std::vector<core::ProcessId>& pids) {
    BankersResult res;

    // Collect resource names
    std::map<std::string, int32_t> work;
    for (const auto& rPair : rm.getResourceTypes()) {
        res.resourceNames.push_back(rPair.first);
        work[rPair.first] = rPair.second.availableInstances;
        res.availableAtStart.push_back(rPair.second.availableInstances);
    }

    // Need matrix: Need = Max - Allocation
    std::map<core::ProcessId, std::map<std::string, int32_t>> need;
    for (core::ProcessId pid : pids) {
        for (const auto& rName : res.resourceNames) {
            int32_t maxC = rm.getMaxClaim(pid, rName);
            int32_t alloc = rm.getAllocation(pid, rName);
            if (alloc > maxC) {
                res.isSafe = false;
                res.explanation = "Inconsistent configuration: Allocation exceeds max claim for PID " + std::to_string(pid);
                return res;
            }
            need[pid][rName] = maxC - alloc;
        }
    }

    std::set<core::ProcessId> finished;

    while (finished.size() < pids.size()) {
        bool found = false;
        for (core::ProcessId pid : pids) {
            if (finished.count(pid) > 0) continue;

            // Check if Need_i <= Work
            bool canSatisfy = true;
            for (const auto& rName : res.resourceNames) {
                if (need[pid][rName] > work[rName]) {
                    canSatisfy = false;
                    break;
                }
            }

            if (canSatisfy) {
                // Process completes and releases allocation back to Work
                for (const auto& rName : res.resourceNames) {
                    work[rName] += rm.getAllocation(pid, rName);
                }
                finished.insert(pid);
                res.safeSequence.push_back(pid);
                found = true;
                break;
            }
        }

        if (!found) {
            // Unsafe state: unable to find any remaining process whose need can be satisfied
            res.isSafe = false;
            std::ostringstream expl;
            expl << "System is in an UNSAFE state. Deadlock could occur.\n"
                 << "Unable to satisfy remaining needs for processes: ";
            for (core::ProcessId pid : pids) {
                if (finished.count(pid) == 0) {
                    expl << "P" << pid << " ";
                }
            }
            res.explanation = expl.str();
            return res;
        }
    }

    res.isSafe = true;
    std::ostringstream expl;
    expl << "Safe state verified. A valid safe execution sequence exists: <";
    for (size_t i = 0; i < res.safeSequence.size(); ++i) {
        expl << "P" << res.safeSequence[i] << (i + 1 < res.safeSequence.size() ? ", " : ">");
    }
    res.explanation = expl.str();
    return res;
}

std::string BankersAlgorithm::formatSafeStateReport(const BankersResult& result) {
    std::ostringstream oss;
    oss << "Banker's Algorithm Safe-State Analysis:\n";
    oss << "Status: " << (result.isSafe ? "[SAFE STATE]" : "[UNSAFE STATE]") << "\n";
    oss << result.explanation << "\n";
    return oss.str();
}

} // namespace deadlock
} // namespace watchos
