#include "watchos/scheduler/aging_analyzer.hpp"
#include "watchos/scheduler/priority_scheduler.hpp"
#include <sstream>
#include <iomanip>

namespace watchos {
namespace scheduler {

AgingComparisonResult AgingAnalyzer::compare(
    const std::vector<process::Process>& processes,
    const core::SystemSettings& settings,
    bool preemptive,
    core::TimeUnits agingInterval,
    core::Priority agingIncrement,
    core::TimeUnits starvationThreshold
) {
    AgingComparisonResult res;
    res.agingInterval = agingInterval;
    res.agingIncrement = agingIncrement;
    res.starvationThreshold = starvationThreshold;

    PriorityScheduler schedWithoutAging(preemptive, false, agingInterval, agingIncrement);
    res.beforeAging = schedWithoutAging.schedule(processes, settings);

    PriorityScheduler schedWithAging(preemptive, true, agingInterval, agingIncrement);
    res.afterAging = schedWithAging.schedule(processes, settings);

    std::ostringstream summary;
    summary << "Aging Analysis (Interval=" << agingInterval << ", Increment=" << agingIncrement << "):\n";
    if (res.afterAging.maxWaitingTime < res.beforeAging.maxWaitingTime) {
        summary << "- Maximum waiting time decreased from " << res.beforeAging.maxWaitingTime
                << " to " << res.afterAging.maxWaitingTime << " time units.\n";
    } else {
        summary << "- Maximum waiting time remained unchanged (" << res.beforeAging.maxWaitingTime << " units).\n";
    }

    if (res.afterAging.starvationBreaches < res.beforeAging.starvationBreaches) {
        summary << "- Starvation breaches reduced from " << res.beforeAging.starvationBreaches
                << " to " << res.afterAging.starvationBreaches << ".\n";
    } else {
        summary << "- Starvation breaches: " << res.afterAging.starvationBreaches << ".\n";
    }

    res.analysisSummary = summary.str();
    return res;
}

std::string AgingAnalyzer::formatComparisonReport(const AgingComparisonResult& result) {
    std::ostringstream oss;
    oss << "+--------------------------------+--------------------+--------------------+\n";
    oss << "| Metric                         | Before Aging       | After Aging        |\n";
    oss << "+--------------------------------+--------------------+--------------------+\n";
    oss << "| Average Waiting Time           | " << std::setw(18) << std::fixed << std::setprecision(2) << result.beforeAging.avgWaitingTime
        << " | " << std::setw(18) << result.afterAging.avgWaitingTime << " |\n";
    oss << "| Maximum Waiting Time           | " << std::setw(18) << result.beforeAging.maxWaitingTime
        << " | " << std::setw(18) << result.afterAging.maxWaitingTime << " |\n";
    oss << "| Average Turnaround Time        | " << std::setw(18) << result.beforeAging.avgTurnaroundTime
        << " | " << std::setw(18) << result.afterAging.avgTurnaroundTime << " |\n";
    oss << "| Processes Exceeding Threshold  | " << std::setw(18) << result.beforeAging.starvationBreaches
        << " | " << std::setw(18) << result.afterAging.starvationBreaches << " |\n";
    oss << "+--------------------------------+--------------------+--------------------+\n";
    oss << "\n" << result.analysisSummary << "\n";
    return oss.str();
}

} // namespace scheduler
} // namespace watchos
