#ifndef WATCHOS_SCHEDULER_SCHEDULING_METRICS_HPP
#define WATCHOS_SCHEDULER_SCHEDULING_METRICS_HPP

#include "watchos/core/types.hpp"
#include <vector>
#include <string>
#include <map>

namespace watchos {
namespace scheduler {

struct GanttSegment {
    core::TimeUnits startTime = 0;
    core::TimeUnits endTime = 0;
    core::ProcessId pid = 0;
    bool isIdle = false;
    bool isContextSwitch = false;
};

struct ProcessScheduleResult {
    core::ProcessId pid = 0;
    core::TimeUnits arrivalTime = 0;
    core::TimeUnits cpuBurst = 0;
    core::Priority priority = 0;
    core::TimeUnits firstStartTime = 0;
    core::TimeUnits completionTime = 0;
    core::TimeUnits turnaroundTime = 0;
    core::TimeUnits waitingTime = 0;
    core::TimeUnits responseTime = 0;
    int32_t contextSwitches = 0;
};

struct AggregateMetrics {
    std::string algorithmName;
    double avgWaitingTime = 0.0;
    double avgTurnaroundTime = 0.0;
    double avgResponseTime = 0.0;
    double throughput = 0.0; // completed processes per unit time
    double cpuUtilization = 0.0; // %
    int32_t totalContextSwitches = 0;
    core::TimeUnits totalElapsedTime = 0;
    core::TimeUnits totalBusyTime = 0;
    core::TimeUnits totalIdleTime = 0;
    int32_t starvationBreaches = 0;
    core::TimeUnits maxWaitingTime = 0;

    std::vector<ProcessScheduleResult> processResults;
    std::vector<GanttSegment> ganttSegments;
};

class MetricsCalculator {
public:
    static AggregateMetrics calculate(
        const std::string& algoName,
        const std::vector<ProcessScheduleResult>& results,
        const std::vector<GanttSegment>& gantt,
        core::TimeUnits starvationThreshold = 10,
        core::TimeUnits contextSwitchOverhead = 0
    );
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_SCHEDULING_METRICS_HPP
