#ifndef WATCHOS_IO_DISK_SCHEDULER_HPP
#define WATCHOS_IO_DISK_SCHEDULER_HPP

#include <vector>
#include <string>
#include <cstdint>

namespace watchos {
namespace io {

enum class DiskAlgorithm {
    FCFS,
    SSTF,
    SCAN,
    CSCAN
};

inline std::string diskAlgorithmToString(DiskAlgorithm algo) {
    switch (algo) {
        case DiskAlgorithm::FCFS: return "FCFS";
        case DiskAlgorithm::SSTF: return "SSTF";
        case DiskAlgorithm::SCAN: return "SCAN";
        case DiskAlgorithm::CSCAN: return "C-SCAN";
        default: return "Unknown";
    }
}

struct DiskScheduleResult {
    DiskAlgorithm algorithm;
    std::string algorithmName;
    int32_t initialHead = 53;
    int32_t minCylinder = 0;
    int32_t maxCylinder = 199;
    std::vector<int32_t> serviceOrder;
    int64_t totalHeadMovement = 0;
    double averageHeadMovement = 0.0;
    std::string direction = "UP"; // "UP" or "DOWN"
};

class DiskScheduler {
public:
    static DiskScheduleResult scheduleFcfs(int32_t initialHead, const std::vector<int32_t>& requests);
    static DiskScheduleResult scheduleSstf(int32_t initialHead, const std::vector<int32_t>& requests);
    static DiskScheduleResult scheduleScan(int32_t initialHead, const std::vector<int32_t>& requests, int32_t maxCylinder = 199, const std::string& direction = "UP");
    static DiskScheduleResult scheduleCscan(int32_t initialHead, const std::vector<int32_t>& requests, int32_t maxCylinder = 199);

    static std::string formatResultReport(const DiskScheduleResult& result);
};

} // namespace io
} // namespace watchos

#endif // WATCHOS_IO_DISK_SCHEDULER_HPP
