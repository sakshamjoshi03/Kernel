#include "watchos/scheduler/srtf.hpp"
#include <algorithm>
#include <map>

namespace watchos {
namespace scheduler {

AggregateMetrics SrtfScheduler::schedule(
    const std::vector<process::Process>& processes,
    const core::SystemSettings& settings
) {
    if (processes.empty()) {
        return AggregateMetrics{};
    }

    struct ProcTrack {
        process::Process proc;
        core::TimeUnits remainingTime = 0;
        core::TimeUnits firstStartTime = -1;
        core::TimeUnits completionTime = -1;
        int32_t contextSwitches = 0;
    };

    std::vector<ProcTrack> trackers;
    for (const auto& p : processes) {
        trackers.push_back({p, p.getCpuBurst(), -1, -1, 0});
    }

    std::vector<GanttSegment> gantt;
    core::TimeUnits currentTime = 0;
    size_t completedCount = 0;
    core::ProcessId currentRunningPid = -1;

    while (completedCount < trackers.size()) {
        // Find ready process with shortest remaining time
        int bestIdx = -1;
        for (size_t i = 0; i < trackers.size(); ++i) {
            if (trackers[i].remainingTime > 0 && trackers[i].proc.getArrivalTime() <= currentTime) {
                if (bestIdx == -1) {
                    bestIdx = static_cast<int>(i);
                } else {
                    if (trackers[i].remainingTime < trackers[bestIdx].remainingTime) {
                        bestIdx = static_cast<int>(i);
                    } else if (trackers[i].remainingTime == trackers[bestIdx].remainingTime) {
                        if (trackers[i].proc.getArrivalTime() < trackers[bestIdx].proc.getArrivalTime()) {
                            bestIdx = static_cast<int>(i);
                        } else if (trackers[i].proc.getArrivalTime() == trackers[bestIdx].proc.getArrivalTime() &&
                                   trackers[i].proc.getPid() < trackers[bestIdx].proc.getPid()) {
                            bestIdx = static_cast<int>(i);
                        }
                    }
                }
            }
        }

        if (bestIdx == -1) {
            // Idle until next arrival
            core::TimeUnits nextArrival = -1;
            for (const auto& t : trackers) {
                if (t.remainingTime > 0) {
                    if (nextArrival == -1 || t.proc.getArrivalTime() < nextArrival) {
                        nextArrival = t.proc.getArrivalTime();
                    }
                }
            }
            if (!gantt.empty() && gantt.back().isIdle) {
                gantt.back().endTime = nextArrival;
            } else {
                gantt.push_back({currentTime, nextArrival, 0, true, false});
            }
            currentTime = nextArrival;
            currentRunningPid = -1;
            continue;
        }

        auto& chosen = trackers[bestIdx];
        if (chosen.firstStartTime < 0) {
            chosen.firstStartTime = currentTime;
        }
        if (currentRunningPid != chosen.proc.getPid()) {
            chosen.contextSwitches++;
            currentRunningPid = chosen.proc.getPid();
        }

        // Determine how long chosen can run before next arrival or completion
        // Find next process arrival strictly greater than currentTime
        core::TimeUnits nextArrival = -1;
        for (const auto& t : trackers) {
            if (t.remainingTime > 0 && t.proc.getArrivalTime() > currentTime) {
                if (nextArrival == -1 || t.proc.getArrivalTime() < nextArrival) {
                    nextArrival = t.proc.getArrivalTime();
                }
            }
        }

        core::TimeUnits timeSlice = chosen.remainingTime;
        if (nextArrival != -1 && (currentTime + chosen.remainingTime) > nextArrival) {
            timeSlice = nextArrival - currentTime;
        }

        core::TimeUnits startTime = currentTime;
        core::TimeUnits endTime = startTime + timeSlice;

        // Merge contiguous segment in Gantt chart if same PID
        if (!gantt.empty() && !gantt.back().isIdle && gantt.back().pid == chosen.proc.getPid()) {
            gantt.back().endTime = endTime;
        } else {
            gantt.push_back({startTime, endTime, chosen.proc.getPid(), false, false});
        }

        chosen.remainingTime -= timeSlice;
        currentTime = endTime;

        if (chosen.remainingTime == 0) {
            chosen.completionTime = currentTime;
            completedCount++;
            currentRunningPid = -1;
        }
    }

    std::vector<ProcessScheduleResult> results;
    for (const auto& t : trackers) {
        ProcessScheduleResult res;
        res.pid = t.proc.getPid();
        res.arrivalTime = t.proc.getArrivalTime();
        res.cpuBurst = t.proc.getCpuBurst();
        res.priority = t.proc.getPriority();
        res.firstStartTime = t.firstStartTime;
        res.completionTime = t.completionTime;
        res.turnaroundTime = res.completionTime - res.arrivalTime;
        res.waitingTime = res.turnaroundTime - res.cpuBurst;
        res.responseTime = res.firstStartTime - res.arrivalTime;
        res.contextSwitches = t.contextSwitches;
        results.push_back(res);
    }

    return MetricsCalculator::calculate(getName(), results, gantt, 10, settings.contextSwitchOverhead);
}

} // namespace scheduler
} // namespace watchos
