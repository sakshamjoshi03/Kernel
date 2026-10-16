#include "watchos/process/workload_validator.hpp"
#include "tests/fixtures/test_workloads.hpp"
#include <iostream>
#include <cassert>

void testValidGoldenWorkload() {
    std::cout << "[TEST] Running testValidGoldenWorkload...\n";
    auto golden = watchos::test::getGoldenWorkload();
    std::string err;
    bool valid = watchos::process::WorkloadValidator::isWorkloadValid(golden, err);
    assert(valid);
    assert(err.empty());
    std::cout << "[PASS] Golden workload is valid.\n";
}

void testDuplicatePidDetection() {
    std::cout << "[TEST] Running testDuplicatePidDetection...\n";
    std::vector<watchos::process::Process> procs;
    procs.emplace_back(101, 0, 5, 1);
    procs.emplace_back(101, 2, 4, 2); // Duplicate PID

    std::string err;
    bool valid = watchos::process::WorkloadValidator::isWorkloadValid(procs, err);
    assert(!valid);
    assert(err.find("Duplicate") != std::string::npos);
    std::cout << "[PASS] Duplicate PID rejected correctly: " << err << "\n";
}

void testNegativeArrivalTime() {
    std::cout << "[TEST] Running testNegativeArrivalTime...\n";
    std::vector<watchos::process::Process> procs;
    procs.emplace_back(101, -3, 5, 1); // Negative AT

    std::string err;
    bool valid = watchos::process::WorkloadValidator::isWorkloadValid(procs, err);
    assert(!valid);
    assert(err.find("Arrival time") != std::string::npos);
    std::cout << "[PASS] Negative arrival time rejected correctly: " << err << "\n";
}

void testZeroOrNegativeBurst() {
    std::cout << "[TEST] Running testZeroOrNegativeBurst...\n";
    std::vector<watchos::process::Process> procs;
    procs.emplace_back(101, 0, 0, 1); // Zero burst

    std::string err;
    bool valid = watchos::process::WorkloadValidator::isWorkloadValid(procs, err);
    assert(!valid);
    assert(err.find("CPU burst") != std::string::npos);
    std::cout << "[PASS] Zero CPU burst rejected correctly: " << err << "\n";
}

void testInvalidIoTiming() {
    std::cout << "[TEST] Running testInvalidIoTiming...\n";
    std::vector<watchos::process::Process> procs;
    watchos::process::Process p(101, 0, 4, 1);
    p.addIoRequest({5, 2, "DISK"}); // Request time 5 >= CPU burst 4
    procs.push_back(p);

    std::string err;
    bool valid = watchos::process::WorkloadValidator::isWorkloadValid(procs, err);
    assert(!valid);
    assert(err.find("I/O request time") != std::string::npos);
    std::cout << "[PASS] Invalid I/O request time rejected correctly: " << err << "\n";
}

void testResourceClaimIntegrity() {
    std::cout << "[TEST] Running testResourceClaimIntegrity...\n";
    std::vector<watchos::process::Process> procs;
    watchos::process::Process p(101, 0, 5, 1);
    p.addResourceClaim({"R1", 2, 4}); // Initial allocation 4 > max claim 2
    procs.push_back(p);

    std::string err;
    bool valid = watchos::process::WorkloadValidator::isWorkloadValid(procs, err);
    assert(!valid);
    assert(err.find("Initial allocation") != std::string::npos);
    std::cout << "[PASS] Initial allocation exceeding max claim rejected correctly: " << err << "\n";
}

int main() {
    try {
        testValidGoldenWorkload();
        testDuplicatePidDetection();
        testNegativeArrivalTime();
        testZeroOrNegativeBurst();
        testInvalidIoTiming();
        testResourceClaimIntegrity();
        std::cout << "All WorkloadValidator unit tests passed successfully!\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Test failed with exception: " << ex.what() << "\n";
        return 1;
    }
}
