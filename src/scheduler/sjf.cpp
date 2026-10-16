#include "watchos/scheduler/sjf.hpp"
#include <algorithm>

namespace watchos {
namespace scheduler {

AggregateMetrics SjfScheduler::schedule(
    const std::vector<process::Process>& processes,
    const core::SystemSettings& settings
) {
    if (processes.empty()) {
        return AggregateMetrics{};
    }

    struct ProcState {
        process::Process proc;
        bool completed = false;
    };

    std::vector<ProcState> stateList;
    for (const auto& p : processes) {
        stateList.push_back({p, false});
    }

    std::vector<GanttSegment> gantt;
    std::vector<ProcessScheduleResult> results;
    core::TimeUnits currentTime = 0;
    size_t completedCount = 0;

    while (completedCount < stateList.size()) {
        // Find arrived, non-completed process with shortest CPU burst
        int bestIdx = -1;
        for (size_t i = 0; i < stateList.size(); ++i) {
            if (stateList[i].completed) continue;
            if (stateList[i].proc.getArrivalTime() <= currentTime) {
                if (bestIdx == -1) {
                    bestIdx = static_cast<int>(i);
                } else {
                    const auto& bestP = stateList[bestIdx].proc;
                    const auto& candP = stateList[i].proc;
                    if (candP.getCpuBurst() < bestP.getCpuBurst()) {
                        bestIdx = static_cast<int>(i);
                    } else if (candP.getCpuBurst() == bestP.getCpuBurst()) {
                        if (candP.getArrivalTime() < bestP.getArrivalTime()) {
                            bestIdx = static_cast<int>(i);
                        } else if (candP.getArrivalTime() == bestP.getArrivalTime() && candP.getPid() < bestP.getPid()) {
                            bestIdx = static_cast<int>(i);
                        }
                    }
                }
            }
        }

        if (bestIdx == -1) {
            // No process ready at currentTime; jump to next earliest arrival
            core::TimeUnits nextArrival = -1;
            for (const auto& item : stateList) {
                if (!item.completed) {
                    if (nextArrival == -1 || item.proc.getArrivalTime() < nextArrival) {
                        nextArrival = item.proc.getArrivalTime();
                    }
                }
            }
            GanttSegment idle;
            idle.startTime = currentTime;
            idle.endTime = nextArrival;
            idle.pid = 0;
            idle.isIdle = true;
            gantt.push_back(idle);
            currentTime = nextArrival;
            continue;
        }

        // Execute chosen process to completion (non-preemptive)
        auto& chosen = stateList[bestIdx];
        core::TimeUnits startTime = currentTime;
        core::TimeUnits endTime = startTime + chosen.proc.getCpuBurst();

        GanttSegment seg;
        seg.startTime = startTime;
        seg.endTime = endTime;
        seg.pid = chosen.proc.getPid();
        seg.isIdle = false;
        gantt.push_back(seg);

        ProcessScheduleResult res;
        res.pid = chosen.proc.getPid();
        res.arrivalTime = chosen.proc.getArrivalTime();
        res.cpuBurst = chosen.proc.getCpuBurst();
        res.priority = chosen.proc.getPriority();
        res.firstStartTime = startTime;
        res.completionTime = endTime;
        res.turnaroundTime = res.completionTime - res.arrivalTime;
        res.waitingTime = res.turnaroundTime - res.cpuBurst;
        res.responseTime = res.firstStartTime - res.arrivalTime;
        res.contextSwitches = 1;
        results.push_back(res);

        chosen.completed = true;
        completedCount++;
        currentTime = endTime;
    }

    std::sort(results.begin(), results.end(), [](const ProcessScheduleResult& a, const ProcessScheduleResult& b) {
        return a.pid < b.pid;
    });

    return MetricsCalculator::calculate(getName(), results, gantt, 10, settings.contextSwitchOverhead);
}

} // namespace scheduler
} // namespace watchos
