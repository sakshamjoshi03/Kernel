#include "watchos/scheduler/fcfs.hpp"
#include "watchos/scheduler/sjf.hpp"
#include "watchos/scheduler/srtf.hpp"
#include "watchos/scheduler/priority_scheduler.hpp"
#include "watchos/scheduler/round_robin.hpp"
#include "watchos/process/process.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

using namespace watchos;

void testSingleProcess() {
    std::cout << "[TEST] CPU Scheduling Edge Case: Single Process...\n";
    core::SystemSettings settings;
    settings.defaultTimeQuantum = 3;

    std::vector<process::Process> procs = {
        process::Process(101, 5, 10, 1) // Arr = 5, Burst = 10
    };

    scheduler::FcfsScheduler fcfs;
    auto resFcfs = fcfs.schedule(procs, settings);
    assert(resFcfs.processResults.size() == 1);
    assert(resFcfs.processResults[0].arrivalTime == 5);
    assert(resFcfs.processResults[0].firstStartTime == 5);
    assert(resFcfs.processResults[0].completionTime == 15);
    assert(resFcfs.processResults[0].waitingTime == 0);
    assert(resFcfs.processResults[0].responseTime == 0);
    assert(resFcfs.processResults[0].turnaroundTime == 10);
    assert(resFcfs.totalContextSwitches == 0);

    // Verify Gantt: [0, 5) Idle, [5, 15) P101
    assert(resFcfs.ganttSegments.size() == 2);
    assert(resFcfs.ganttSegments[0].isIdle && resFcfs.ganttSegments[0].startTime == 0 && resFcfs.ganttSegments[0].endTime == 5);
    assert(!resFcfs.ganttSegments[1].isIdle && resFcfs.ganttSegments[1].pid == 101 && resFcfs.ganttSegments[1].startTime == 5 && resFcfs.ganttSegments[1].endTime == 15);

    // Verify Round Robin with quantum 3: 10 / 3 = 4 slices (3, 3, 3, 1)
    scheduler::RoundRobinScheduler rr(3);
    auto resRr = rr.schedule(procs, settings);
    assert(resRr.processResults[0].completionTime == 15);
    assert(resRr.processResults[0].waitingTime == 0);
    assert(resRr.processResults[0].responseTime == 0);

    std::cout << "  -> Single process verified across FCFS and Round Robin.\n";
}

void testSimultaneousArrivalsDeterministicTieBreaking() {
    std::cout << "[TEST] CPU Scheduling Edge Case: Simultaneous Arrivals & Deterministic Tie-Breaking...\n";
    core::SystemSettings settings;

    // Three processes arriving at t=0 with identical bursts
    std::vector<process::Process> procs = {
        process::Process(300, 0, 4, 1),
        process::Process(100, 0, 4, 1),
        process::Process(200, 0, 4, 1)
    };

    scheduler::FcfsScheduler fcfs;
    auto res = fcfs.schedule(procs, settings);

    // Tie-breaker must be ascending PID order: 100, 200, 300
    // Verify execution order in Gantt
    assert(res.ganttSegments.size() == 3);
    assert(res.ganttSegments[0].pid == 100 && res.ganttSegments[0].startTime == 0 && res.ganttSegments[0].endTime == 4);
    assert(res.ganttSegments[1].pid == 200 && res.ganttSegments[1].startTime == 4 && res.ganttSegments[1].endTime == 8);
    assert(res.ganttSegments[2].pid == 300 && res.ganttSegments[2].startTime == 8 && res.ganttSegments[2].endTime == 12);

    // Results vector should be sorted by PID
    assert(res.processResults[0].pid == 100 && res.processResults[0].waitingTime == 0);
    assert(res.processResults[1].pid == 200 && res.processResults[1].waitingTime == 4);
    assert(res.processResults[2].pid == 300 && res.processResults[2].waitingTime == 8);

    std::cout << "  -> Deterministic tie-breaking by PID verified.\n";
}

void testCpuIdlePeriodsAndNonOverlappingGantt() {
    std::cout << "[TEST] CPU Scheduling Edge Case: CPU Idle Periods and Non-Overlapping Gantt...\n";
    core::SystemSettings settings;

    std::vector<process::Process> procs = {
        process::Process(1, 0, 3, 1), // Executes 0..3
        process::Process(2, 8, 4, 1)  // Arrives at 8; CPU idle 3..8
    };

    scheduler::FcfsScheduler fcfs;
    auto res = fcfs.schedule(procs, settings);

    // Segments: [0, 3) P1, [3, 8) Idle, [8, 12) P2
    assert(res.ganttSegments.size() == 3);
    assert(!res.ganttSegments[0].isIdle && res.ganttSegments[0].pid == 1 && res.ganttSegments[0].startTime == 0 && res.ganttSegments[0].endTime == 3);
    assert(res.ganttSegments[1].isIdle && res.ganttSegments[1].startTime == 3 && res.ganttSegments[1].endTime == 8);
    assert(!res.ganttSegments[2].isIdle && res.ganttSegments[2].pid == 2 && res.ganttSegments[2].startTime == 8 && res.ganttSegments[2].endTime == 12);

    // Check non-overlapping guarantee
    for (size_t i = 1; i < res.ganttSegments.size(); ++i) {
        assert(res.ganttSegments[i].startTime == res.ganttSegments[i - 1].endTime);
    }

    // CPU utilization = (3 + 4) / 12 = 7 / 12 * 100% = 58.33%
    double expectedUtil = (7.0 / 12.0) * 100.0;
    assert(std::abs(res.cpuUtilization - expectedUtil) < 0.1);

    std::cout << "  -> Idle segments and non-overlapping timeline verified.\n";
}

void testPreemptiveVsNonPreemptivePriority() {
    std::cout << "[TEST] CPU Scheduling Edge Case: Preemptive vs Non-Preemptive Priority...\n";
    core::SystemSettings settings;

    // P1 arrives at 0 with low priority (pri=5), burst=10
    // P2 arrives at 2 with high priority (pri=1), burst=3
    std::vector<process::Process> procs = {
        process::Process(1, 0, 10, 5),
        process::Process(2, 2, 3, 1)
    };

    // Non-preemptive: P1 runs to completion (0..10), then P2 (10..13)
    scheduler::PriorityScheduler nonPreempt(false);
    auto resNp = nonPreempt.schedule(procs, settings);
    assert(resNp.processResults[0].pid == 1 && resNp.processResults[0].completionTime == 10);
    assert(resNp.processResults[1].pid == 2 && resNp.processResults[1].completionTime == 13);
    assert(resNp.processResults[1].waitingTime == 8); // 10 - 2

    // Preemptive: P1 runs 0..2, preempted by P2 at 2, P2 runs 2..5, P1 resumes 5..13
    scheduler::PriorityScheduler preempt(true);
    auto resP = preempt.schedule(procs, settings);
    assert(resP.processResults[1].pid == 2 && resP.processResults[1].completionTime == 5);
    assert(resP.processResults[1].waitingTime == 0); // Started immediately upon arrival!
    assert(resP.processResults[0].pid == 1 && resP.processResults[0].completionTime == 13);
    assert(resP.processResults[0].waitingTime == 3); // Preempted for 3 units

    std::cout << "  -> Preemption at arrival boundary verified successfully.\n";
}

void testEmptyWorkloadReturnsZeroMetrics() {
    std::cout << "[TEST] CPU Scheduling Edge Case: Empty Workload...\n";
    core::SystemSettings settings;
    std::vector<process::Process> emptyProcs;

    scheduler::FcfsScheduler fcfs;
    auto r1 = fcfs.schedule(emptyProcs, settings);
    assert(r1.processResults.empty());
    assert(r1.avgWaitingTime == 0.0);

    scheduler::SjfScheduler sjf;
    auto r2 = sjf.schedule(emptyProcs, settings);
    assert(r2.processResults.empty());

    scheduler::SrtfScheduler srtf;
    auto r3 = srtf.schedule(emptyProcs, settings);
    assert(r3.processResults.empty());

    scheduler::RoundRobinScheduler rr(2);
    auto r4 = rr.schedule(emptyProcs, settings);
    assert(r4.processResults.empty());

    std::cout << "  -> Empty workload handled without exceptions or division-by-zero.\n";
}

int main() {
    std::cout << "============================================================\n";
    std::cout << "       WATCH OS — CPU SCHEDULING CORRECTNESS & EDGE CASES\n";
    std::cout << "============================================================\n";

    testSingleProcess();
    testSimultaneousArrivalsDeterministicTieBreaking();
    testCpuIdlePeriodsAndNonOverlappingGantt();
    testPreemptiveVsNonPreemptivePriority();
    testEmptyWorkloadReturnsZeroMetrics();

    std::cout << "All CPU scheduling edge case tests PASSED successfully!\n";
    return 0;
}
