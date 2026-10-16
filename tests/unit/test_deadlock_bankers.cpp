#include "watchos/deadlock/resource_manager.hpp"
#include "watchos/deadlock/bankers_algorithm.hpp"
#include "watchos/deadlock/deadlock_detector.hpp"
#include <iostream>
#include <cassert>

void testBankersSafeState() {
    std::cout << "[TEST] Running testBankersSafeState...\n";
    // Standard 3-resource Banker's problem
    // Resources: A=10, B=5, C=7
    watchos::deadlock::ResourceManager rm;
    rm.registerResourceType("A", 10);
    rm.registerResourceType("B", 5);
    rm.registerResourceType("C", 7);

    // Processes 0, 1, 2
    // P0: Alloc(0, 1, 0), Max(7, 5, 3)
    // P1: Alloc(2, 0, 0), Max(3, 2, 2)
    // P2: Alloc(3, 0, 2), Max(9, 0, 2)
    // Total Alloc: A=5, B=1, C=2
    // Available: A=5, B=4, C=5
    rm.setAllocation(100, "A", 0); rm.setAllocation(100, "B", 1); rm.setAllocation(100, "C", 0);
    rm.setMaxClaim(100, "A", 7);   rm.setMaxClaim(100, "B", 5);   rm.setMaxClaim(100, "C", 3);

    rm.setAllocation(101, "A", 2); rm.setAllocation(101, "B", 0); rm.setAllocation(101, "C", 0);
    rm.setMaxClaim(101, "A", 3);   rm.setMaxClaim(101, "B", 2);   rm.setMaxClaim(101, "C", 2);

    rm.setAllocation(102, "A", 3); rm.setAllocation(102, "B", 0); rm.setAllocation(102, "C", 2);
    rm.setMaxClaim(102, "A", 9);   rm.setMaxClaim(102, "B", 0);   rm.setMaxClaim(102, "C", 2);

    rm.getResourceTypes()["A"].availableInstances = 5;
    rm.getResourceTypes()["B"].availableInstances = 4;
    rm.getResourceTypes()["C"].availableInstances = 5;

    std::vector<watchos::core::ProcessId> pids = {100, 101, 102};
    auto result = watchos::deadlock::BankersAlgorithm::evaluateSafeState(rm, pids);

    assert(result.isSafe);
    assert(!result.safeSequence.empty());
    assert(result.safeSequence.size() == 3);
    std::cout << "[PASS] Banker's algorithm found safe sequence: " << result.explanation << "\n";
}

void testDeadlockDetection() {
    std::cout << "[TEST] Running testDeadlockDetection...\n";
    // Circular wait deadlock:
    // P1 owns R1, requests R2
    // P2 owns R2, requests R1
    // Available: R1=0, R2=0
    watchos::deadlock::ResourceManager rm;
    rm.registerResourceType("R1", 1);
    rm.registerResourceType("R2", 1);
    rm.getResourceTypes()["R1"].availableInstances = 0;
    rm.getResourceTypes()["R2"].availableInstances = 0;

    rm.setAllocation(1, "R1", 1);
    rm.setRequest(1, "R2", 1);

    rm.setAllocation(2, "R2", 1);
    rm.setRequest(2, "R1", 1);

    std::vector<watchos::core::ProcessId> pids = {1, 2};
    auto result = watchos::deadlock::DeadlockDetector::detect(rm, pids);

    assert(result.isDeadlocked);
    assert(result.deadlockedProcesses.size() == 2);
    std::cout << "[PASS] Deadlock correctly identified in circular wait scenario.\n";
}

int main() {
    try {
        testBankersSafeState();
        testDeadlockDetection();
        std::cout << "All Deadlock & Banker's Algorithm unit tests passed successfully!\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Test failed with exception: " << ex.what() << "\n";
        return 1;
    }
}
