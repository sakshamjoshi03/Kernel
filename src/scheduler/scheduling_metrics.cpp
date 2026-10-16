#include "watchos/scheduler/scheduling_metrics.hpp"
#include <algorithm>

namespace watchos {
namespace scheduler {

AggregateMetrics MetricsCalculator::calculate(
    const std::string& algoName,
    const std::vector<ProcessScheduleResult>& results,
    const std::vector<GanttSegment>& gantt,
    core::TimeUnits starvationThreshold,
    core::TimeUnits contextSwitchOverhead
) {
    (void)contextSwitchOverhead;
    AggregateMetrics agg;
    agg.algorithmName = algoName;
    agg.processResults = results;
    agg.ganttSegments = gantt;

    std::sort(agg.processResults.begin(), agg.processResults.end(), [](const ProcessScheduleResult& a, const ProcessScheduleResult& b) {
        return a.pid < b.pid;
    });

    if (results.empty()) {
        return agg;
    }

    double sumWt = 0.0, sumTat = 0.0, sumRt = 0.0;
    core::TimeUnits maxWt = 0;
    int32_t starvCount = 0;

    for (const auto& r : results) {
        sumWt += r.waitingTime;
        sumTat += r.turnaroundTime;
        sumRt += r.responseTime;
        if (r.waitingTime > maxWt) {
            maxWt = r.waitingTime;
        }
        if (r.waitingTime >= starvationThreshold) {
            starvCount++;
        }
    }

    agg.avgWaitingTime = sumWt / results.size();
    agg.avgTurnaroundTime = sumTat / results.size();
    agg.avgResponseTime = sumRt / results.size();
    agg.maxWaitingTime = maxWt;
    agg.starvationBreaches = starvCount;

    core::TimeUnits firstTime = 0;
    core::TimeUnits lastTime = 0;
    core::TimeUnits busyTime = 0;
    core::TimeUnits idleTime = 0;
    int32_t switches = 0;
    core::ProcessId lastActivePid = -1;

    if (!gantt.empty()) {
        firstTime = gantt.front().startTime;
        lastTime = gantt.back().endTime;

        for (const auto& seg : gantt) {
            core::TimeUnits duration = seg.endTime - seg.startTime;
            if (seg.isIdle) {
                idleTime += duration;
            } else {
                busyTime += duration;
                if (lastActivePid != -1 && lastActivePid != seg.pid) {
                    switches++;
                }
                lastActivePid = seg.pid;
            }
        }
    }

    agg.totalElapsedTime = lastTime - firstTime;
    agg.totalBusyTime = busyTime;
    agg.totalIdleTime = idleTime;
    agg.totalContextSwitches = switches;

    if (agg.totalElapsedTime > 0) {
        agg.cpuUtilization = (static_cast<double>(busyTime) / agg.totalElapsedTime) * 100.0;
        agg.throughput = static_cast<double>(results.size()) / agg.totalElapsedTime;
    } else {
        agg.cpuUtilization = 100.0;
        agg.throughput = 0.0;
    }

    return agg;
}

} // namespace scheduler
} // namespace watchos
