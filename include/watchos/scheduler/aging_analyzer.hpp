#ifndef WATCHOS_SCHEDULER_AGING_ANALYZER_HPP
#define WATCHOS_SCHEDULER_AGING_ANALYZER_HPP

#include "watchos/scheduler/scheduling_metrics.hpp"
#include "watchos/process/process.hpp"
#include <vector>
#include <string>

namespace watchos {
namespace scheduler {

struct AgingComparisonResult {
    AggregateMetrics beforeAging;
    AggregateMetrics afterAging;
    core::TimeUnits agingInterval = 5;
    core::Priority agingIncrement = 1;
    core::TimeUnits starvationThreshold = 10;
    std::string analysisSummary;
};

class AgingAnalyzer {
public:
    static AgingComparisonResult compare(
        const std::vector<process::Process>& processes,
        const core::SystemSettings& settings,
        bool preemptive = false,
        core::TimeUnits agingInterval = 5,
        core::Priority agingIncrement = 1,
        core::TimeUnits starvationThreshold = 10
    );

    static std::string formatComparisonReport(const AgingComparisonResult& result);
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_AGING_ANALYZER_HPP
