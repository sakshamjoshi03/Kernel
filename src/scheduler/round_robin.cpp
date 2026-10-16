#include "watchos/scheduler/round_robin.hpp"
#include <deque>
#include <algorithm>

namespace watchos {
namespace scheduler {

AggregateMetrics RoundRobinScheduler::schedule(
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
        bool hasArrived = false;
    };

    std::vector<ProcTrack> trackers;
    for (const auto& p : processes) {
        trackers.push_back({p, p.getCpuBurst(), -1, -1, 0, false});
    }

    // Sort original by arrival, then PID
    std::sort(trackers.begin(), trackers.end(), [](const ProcTrack& a, const ProcTrack& b) {
        if (a.proc.getArrivalTime() != b.proc.getArrivalTime()) {
            return a.proc.getArrivalTime() < b.proc.getArrivalTime();
        }
        return a.proc.getPid() < b.proc.getPid();
    });

    std::deque<size_t> readyQueue;
    std::vector<GanttSegment> gantt;
    core::TimeUnits currentTime = 0;
    size_t completedCount = 0;
    core::TimeUnits quantum = (timeQuantum_ > 0) ? timeQuantum_ : 2;

    // Check arrivals at time 0
    for (size_t i = 0; i < trackers.size(); ++i) {
        if (!trackers[i].hasArrived && trackers[i].proc.getArrivalTime() <= currentTime) {
            readyQueue.push_back(i);
            trackers[i].hasArrived = true;
        }
    }

    while (completedCount < trackers.size()) {
        if (readyQueue.empty()) {
            // Find next arriving process
            core::TimeUnits nextArrival = -1;
            for (const auto& t : trackers) {
                if (!t.hasArrived) {
                    if (nextArrival == -1 || t.proc.getArrivalTime() < nextArrival) {
                        nextArrival = t.proc.getArrivalTime();
                    }
                }
            }
            if (nextArrival > currentTime) {
                gantt.push_back({currentTime, nextArrival, 0, true, false});
                currentTime = nextArrival;
            }
            for (size_t i = 0; i < trackers.size(); ++i) {
                if (!trackers[i].hasArrived && trackers[i].proc.getArrivalTime() <= currentTime) {
                    readyQueue.push_back(i);
                    trackers[i].hasArrived = true;
                }
            }
            continue;
        }

        size_t idx = readyQueue.front();
        readyQueue.pop_front();
        auto& chosen = trackers[idx];

        if (chosen.firstStartTime < 0) {
            chosen.firstStartTime = currentTime;
        }
        chosen.contextSwitches++;

        core::TimeUnits runSlice = std::min(chosen.remainingTime, quantum);
        core::TimeUnits startTime = currentTime;
        core::TimeUnits endTime = startTime + runSlice;

        gantt.push_back({startTime, endTime, chosen.proc.getPid(), false, false});

        chosen.remainingTime -= runSlice;
        currentTime = endTime;

        // Check new arrivals during this execution interval [startTime+1, endTime]
        for (size_t i = 0; i < trackers.size(); ++i) {
            if (!trackers[i].hasArrived && trackers[i].proc.getArrivalTime() <= currentTime) {
                readyQueue.push_back(i);
                trackers[i].hasArrived = true;
            }
        }

        if (chosen.remainingTime > 0) {
            // Re-queue process
            readyQueue.push_back(idx);
        } else {
            chosen.completionTime = currentTime;
            completedCount++;
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
