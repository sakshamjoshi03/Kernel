#include "watchos/scheduler/fcfs.hpp"
#include "watchos/scheduler/sjf.hpp"
#include "watchos/scheduler/srtf.hpp"
#include "watchos/scheduler/priority_scheduler.hpp"
#include "watchos/scheduler/round_robin.hpp"
#include "watchos/scheduler/recommendation_engine.hpp"
#include "tests/fixtures/test_workloads.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

void testFcfsOnGoldenWorkload() {
    std::cout << "[TEST] Running testFcfsOnGoldenWorkload...\n";
    // Golden workload:
    // P1: AT=0, BT=5, Pri=2
    // P2: AT=1, BT=3, Pri=1
    // P3: AT=2, BT=1, Pri=3
    // Execution:
    // P1: 0 -> 5 (CT=5, TAT=5, WT=0, RT=0)
    // P2: 5 -> 8 (CT=8, TAT=7, WT=4, RT=4)
    // P3: 8 -> 9 (CT=9, TAT=7, WT=6, RT=6)
    // Total WT = 0 + 4 + 6 = 10, Avg WT = 10 / 3 = 3.33
    // Total TAT = 5 + 7 + 7 = 19, Avg TAT = 19 / 3 = 6.33
    auto procs = watchos::test::getGoldenWorkload();
    watchos::scheduler::FcfsScheduler fcfs;
    watchos::core::SystemSettings settings;

    auto metrics = fcfs.schedule(procs, settings);
    assert(metrics.processResults.size() == 3);

    assert(metrics.processResults[0].completionTime == 5);
    assert(metrics.processResults[0].waitingTime == 0);
    assert(metrics.processResults[0].responseTime == 0);

    assert(metrics.processResults[1].completionTime == 8);
    assert(metrics.processResults[1].waitingTime == 4);
    assert(metrics.processResults[1].responseTime == 4);

    assert(metrics.processResults[2].completionTime == 9);
    assert(metrics.processResults[2].waitingTime == 6);
    assert(metrics.processResults[2].responseTime == 6);

    assert(std::abs(metrics.avgWaitingTime - 3.333) < 0.01);
    assert(std::abs(metrics.avgTurnaroundTime - 6.333) < 0.01);

    std::cout << "[PASS] FCFS golden workload deterministic values match expected calculations.\n";
}

void testSjfOnGoldenWorkload() {
    std::cout << "[TEST] Running testSjfOnGoldenWorkload...\n";
    // Non-preemptive SJF:
    // At T=0, only P1 arrived. P1 runs 0 -> 5.
    // At T=5, P2 (BT=3) and P3 (BT=1) are ready. Shortest is P3 (BT=1).
    // P3 runs 5 -> 6 (CT=6, TAT=4, WT=3, RT=3)
    // P2 runs 6 -> 9 (CT=9, TAT=8, WT=5, RT=5)
    // Total WT = 0 + 3 + 5 = 8, Avg WT = 8 / 3 = 2.67
    auto procs = watchos::test::getGoldenWorkload();
    watchos::scheduler::SjfScheduler sjf;
    watchos::core::SystemSettings settings;

    auto metrics = sjf.schedule(procs, settings);
    assert(metrics.processResults[0].completionTime == 5);
    assert(metrics.processResults[2].completionTime == 6); // P3 completed before P2!
    assert(metrics.processResults[1].completionTime == 9);

    assert(std::abs(metrics.avgWaitingTime - 2.666) < 0.01);
    std::cout << "[PASS] SJF executed shortest available job first deterministically.\n";
}

void testSrtfPreemption() {
    std::cout << "[TEST] Running testSrtfPreemption...\n";
    // P1: AT=0, BT=5. At T=1, P1 rem=4. P2 arrives with BT=3. 3 < 4, so P1 preempted!
    // P2 runs 1 -> 2. At T=2, P3 arrives with BT=1. 1 < 2, so P2 preempted!
    // P3 runs 2 -> 3. P3 finishes at 3.
    // P2 rem=2. Runs 3 -> 5. P2 finishes at 5.
    // P1 rem=4. Runs 5 -> 9. P1 finishes at 9.
    auto procs = watchos::test::getGoldenWorkload();
    watchos::scheduler::SrtfScheduler srtf;
    watchos::core::SystemSettings settings;

    auto metrics = srtf.schedule(procs, settings);
    assert(metrics.processResults[2].completionTime == 3); // P3 finished at 3
    assert(metrics.processResults[1].completionTime == 5); // P2 finished at 5
    assert(metrics.processResults[0].completionTime == 9); // P1 finished at 9

    assert(metrics.processResults[1].firstStartTime == 1);
    assert(metrics.processResults[2].firstStartTime == 2);

    std::cout << "[PASS] SRTF preemption and smallest remaining time order verified.\n";
}

void testRoundRobinQuantum() {
    std::cout << "[TEST] Running testRoundRobinQuantum...\n";
    auto procs = watchos::test::getGoldenWorkload();
    watchos::scheduler::RoundRobinScheduler rr(2); // Q=2
    watchos::core::SystemSettings settings;

    auto metrics = rr.schedule(procs, settings);
    assert(metrics.processResults.size() == 3);
    assert(metrics.totalContextSwitches > 1);
    assert(metrics.cpuUtilization == 100.0);
    std::cout << "[PASS] Round Robin quantum execution verified.\n";
}

int main() {
    try {
        testFcfsOnGoldenWorkload();
        testSjfOnGoldenWorkload();
        testSrtfPreemption();
        testRoundRobinQuantum();
        std::cout << "All CPU Scheduler unit tests passed successfully!\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Test failed with exception: " << ex.what() << "\n";
        return 1;
    }
}
