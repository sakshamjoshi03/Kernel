#include "watchos/scheduler/fcfs.hpp"
#include <algorithm>

namespace watchos {
namespace scheduler {

AggregateMetrics FcfsScheduler::schedule(
    const std::vector<process::Process>& processes,
    const core::SystemSettings& settings
) {
    if (processes.empty()) {
        return AggregateMetrics{};
    }

    // Sort by arrival time, then PID tie-breaker
    std::vector<process::Process> procs = processes;
    std::sort(procs.begin(), procs.end(), [](const process::Process& a, const process::Process& b) {
        if (a.getArrivalTime() != b.getArrivalTime()) {
            return a.getArrivalTime() < b.getArrivalTime();
        }
        return a.getPid() < b.getPid();
    });

    std::vector<GanttSegment> gantt;
    std::vector<ProcessScheduleResult> results;
    core::TimeUnits currentTime = 0;

    for (const auto& p : procs) {
        // Idle period
        if (currentTime < p.getArrivalTime()) {
            GanttSegment idle;
            idle.startTime = currentTime;
            idle.endTime = p.getArrivalTime();
            idle.pid = 0;
            idle.isIdle = true;
            gantt.push_back(idle);
            currentTime = p.getArrivalTime();
        }

        core::TimeUnits startTime = currentTime;
        core::TimeUnits endTime = startTime + p.getCpuBurst();

        GanttSegment seg;
        seg.startTime = startTime;
        seg.endTime = endTime;
        seg.pid = p.getPid();
        seg.isIdle = false;
        gantt.push_back(seg);

        ProcessScheduleResult res;
        res.pid = p.getPid();
        res.arrivalTime = p.getArrivalTime();
        res.cpuBurst = p.getCpuBurst();
        res.priority = p.getPriority();
        res.firstStartTime = startTime;
        res.completionTime = endTime;
        res.turnaroundTime = res.completionTime - res.arrivalTime;
        res.waitingTime = res.turnaroundTime - res.cpuBurst;
        res.responseTime = res.firstStartTime - res.arrivalTime;
        res.contextSwitches = 1;
        results.push_back(res);

        currentTime = endTime;
    }

    return MetricsCalculator::calculate(getName(), results, gantt, 10, settings.contextSwitchOverhead);
}

} // namespace scheduler
} // namespace watchos
