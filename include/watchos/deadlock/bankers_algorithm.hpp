#ifndef WATCHOS_DEADLOCK_BANKERS_ALGORITHM_HPP
#define WATCHOS_DEADLOCK_BANKERS_ALGORITHM_HPP

#include "watchos/deadlock/resource_manager.hpp"
#include <vector>
#include <string>

namespace watchos {
namespace deadlock {

struct BankersResult {
    bool isSafe = false;
    std::vector<core::ProcessId> safeSequence;
    std::string explanation;
    std::vector<std::string> resourceNames;
    std::vector<int32_t> availableAtStart;
};

class BankersAlgorithm {
public:
    static BankersResult evaluateSafeState(const ResourceManager& rm, const std::vector<core::ProcessId>& pids);

    static std::string formatSafeStateReport(const BankersResult& result);
};

} // namespace deadlock
} // namespace watchos

#endif // WATCHOS_DEADLOCK_BANKERS_ALGORITHM_HPP
