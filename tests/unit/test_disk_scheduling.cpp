#include "watchos/io/disk_scheduler.hpp"
#include <iostream>
#include <cassert>

void testDiskSchedulingAlgorithms() {
    std::cout << "[TEST] Running testDiskSchedulingAlgorithms...\n";
    std::vector<int32_t> requests = {98, 183, 37, 122, 14, 124, 65, 67};
    int32_t head = 53;

    auto fcfs = watchos::io::DiskScheduler::scheduleFcfs(head, requests);
    auto sstf = watchos::io::DiskScheduler::scheduleSstf(head, requests);
    auto scan = watchos::io::DiskScheduler::scheduleScan(head, requests, 199, "UP");

    std::cout << "  FCFS Total Movement: " << fcfs.totalHeadMovement << "\n";
    std::cout << "  SSTF Total Movement: " << sstf.totalHeadMovement << "\n";
    std::cout << "  SCAN Total Movement: " << scan.totalHeadMovement << "\n";

    // Standard textbook calculations:
    // FCFS: |98-53| + |183-98| + |37-183| + |122-37| + |14-122| + |124-14| + |65-124| + |67-65| = 640
    assert(fcfs.totalHeadMovement == 640);

    // SSTF seek sequence: 53 -> 65 -> 67 -> 37 -> 14 -> 98 -> 122 -> 124 -> 183 = 236
    assert(sstf.totalHeadMovement == 236);

    // SSTF minimizes head movement significantly compared to FCFS
    assert(sstf.totalHeadMovement < fcfs.totalHeadMovement);

    std::cout << "[PASS] Disk scheduling FCFS (640) and SSTF (236) movements match exact calculations.\n";
}

int main() {
    try {
        testDiskSchedulingAlgorithms();
        std::cout << "All Disk Scheduling unit tests passed successfully!\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Test failed with exception: " << ex.what() << "\n";
        return 1;
    }
}
