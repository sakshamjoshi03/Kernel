#include "watchos/deadlock/deadlock_frequency.hpp"
#include "watchos/deadlock/deadlock_detector.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace watchos {
namespace deadlock {

FrequencyResult DeadlockFrequencyEstimator::estimate(
    const ResourceManager& baseRm,
    const std::vector<core::ProcessId>& pids,
    int32_t trials,
    uint64_t seed
) {
    FrequencyResult res;
    res.randomSeed = seed;
    res.totalTrials = std::max(1, trials);

    std::mt19937_64 rng(seed);

    std::vector<std::string> rNames;
    for (const auto& rPair : baseRm.getResourceTypes()) {
        rNames.push_back(rPair.first);
    }

    for (int t = 0; t < res.totalTrials; ++t) {
        // Create randomized trial scenario: processes request remaining needed resources in random order
        ResourceManager trialRm = baseRm;

        for (core::ProcessId pid : pids) {
            for (const auto& rName : rNames) {
                int maxC = baseRm.getMaxClaim(pid, rName);
                int alloc = baseRm.getAllocation(pid, rName);
                int remainingNeed = std::max(0, maxC - alloc);
                if (remainingNeed > 0) {
                    std::uniform_int_distribution<int> dist(0, remainingNeed);
                    int req = dist(rng);
                    trialRm.setRequest(pid, rName, req);
                }
            }
        }

        auto det = DeadlockDetector::detect(trialRm, pids);
        if (det.isDeadlocked) {
            res.deadlockedTrials++;
            for (core::ProcessId pid : det.deadlockedProcesses) {
                res.processInvolvementCount[pid]++;
            }
        }
    }

    res.empiricalFrequency = static_cast<double>(res.deadlockedTrials) / res.totalTrials;

    std::ostringstream oss;
    oss << "Monte Carlo Deadlock Risk Estimation (Seed=" << seed << ", Trials=" << res.totalTrials << "):\n";
    oss << "+-----------------------+--------------------+\n";
    oss << "| Metric                | Value              |\n";
    oss << "+-----------------------+--------------------+\n";
    oss << "| Total Trials          | " << std::setw(18) << res.totalTrials << " |\n";
    oss << "| Deadlocked Trials     | " << std::setw(18) << res.deadlockedTrials << " |\n";
    oss << "| Empirical Frequency   | " << std::setw(17) << std::fixed << std::setprecision(2) << (res.empiricalFrequency * 100.0) << "% |\n";
    oss << "+-----------------------+--------------------+\n\n";

    oss << "Per-Process Deadlock Involvement:\n";
    for (core::ProcessId pid : pids) {
        int cnt = res.processInvolvementCount[pid];
        double pct = (static_cast<double>(cnt) / res.totalTrials) * 100.0;
        oss << "  PID " << pid << ": Involved in " << cnt << " deadlocked trials (" << std::fixed << std::setprecision(1) << pct << "%)\n";
    }

    res.report = oss.str();
    return res;
}

} // namespace deadlock
} // namespace watchos
