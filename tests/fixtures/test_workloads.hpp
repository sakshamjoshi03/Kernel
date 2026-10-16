#ifndef WATCHOS_TEST_WORKLOADS_HPP
#define WATCHOS_TEST_WORKLOADS_HPP

#include "watchos/process/process.hpp"
#include <vector>

namespace watchos {
namespace test {

inline std::vector<process::Process> getGoldenWorkload() {
    // PRD Section 11.3 Golden test case:
    // P1: AT=0, BT=5, Priority=2
    // P2: AT=1, BT=3, Priority=1
    // P3: AT=2, BT=1, Priority=3
    std::vector<process::Process> procs;

    process::Process p1(1, 0, 5, 2);
    p1.setMemoryConfig({true, 128, {}});
    p1.addIoRequest({2, 3, "DISK"});

    process::Process p2(2, 1, 3, 1);
    p2.setMemoryConfig({true, 64, {}});

    process::Process p3(3, 2, 1, 3);

    procs.push_back(p1);
    procs.push_back(p2);
    procs.push_back(p3);

    return procs;
}

} // namespace test
} // namespace watchos

#endif // WATCHOS_TEST_WORKLOADS_HPP
