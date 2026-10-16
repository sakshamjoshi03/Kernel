#include "watchos/io/disk_scheduler.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace watchos {
namespace io {

DiskScheduleResult DiskScheduler::scheduleFcfs(int32_t initialHead, const std::vector<int32_t>& requests) {
    DiskScheduleResult res;
    res.algorithm = DiskAlgorithm::FCFS;
    res.algorithmName = "FCFS";
    res.initialHead = initialHead;
    res.serviceOrder = requests;

    int32_t currentHead = initialHead;
    for (int32_t req : requests) {
        res.totalHeadMovement += std::abs(req - currentHead);
        currentHead = req;
    }

    if (!requests.empty()) {
        res.averageHeadMovement = static_cast<double>(res.totalHeadMovement) / requests.size();
    }
    return res;
}

DiskScheduleResult DiskScheduler::scheduleSstf(int32_t initialHead, const std::vector<int32_t>& requests) {
    DiskScheduleResult res;
    res.algorithm = DiskAlgorithm::SSTF;
    res.algorithmName = "SSTF";
    res.initialHead = initialHead;

    std::vector<int32_t> pending = requests;
    int32_t currentHead = initialHead;

    while (!pending.empty()) {
        int bestIdx = 0;
        int32_t minDistance = std::abs(pending[0] - currentHead);

        for (size_t i = 1; i < pending.size(); ++i) {
            int32_t dist = std::abs(pending[i] - currentHead);
            if (dist < minDistance) {
                minDistance = dist;
                bestIdx = static_cast<int>(i);
            }
        }

        int32_t chosen = pending[bestIdx];
        res.serviceOrder.push_back(chosen);
        res.totalHeadMovement += minDistance;
        currentHead = chosen;
        pending.erase(pending.begin() + bestIdx);
    }

    if (!requests.empty()) {
        res.averageHeadMovement = static_cast<double>(res.totalHeadMovement) / requests.size();
    }
    return res;
}

DiskScheduleResult DiskScheduler::scheduleScan(int32_t initialHead, const std::vector<int32_t>& requests, int32_t maxCylinder, const std::string& direction) {
    DiskScheduleResult res;
    res.algorithm = DiskAlgorithm::SCAN;
    res.algorithmName = "SCAN (Elevator)";
    res.initialHead = initialHead;
    res.maxCylinder = maxCylinder;
    res.direction = direction;

    std::vector<int32_t> left, right;
    for (int32_t req : requests) {
        if (req < initialHead) left.push_back(req);
        else right.push_back(req);
    }

    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    int32_t currentHead = initialHead;

    if (direction == "UP") {
        for (int32_t req : right) {
            res.serviceOrder.push_back(req);
            res.totalHeadMovement += std::abs(req - currentHead);
            currentHead = req;
        }
        if (!left.empty()) {
            // Seek to cylinder end if there are left requests
            res.totalHeadMovement += std::abs(maxCylinder - currentHead);
            currentHead = maxCylinder;

            std::reverse(left.begin(), left.end());
            for (int32_t req : left) {
                res.serviceOrder.push_back(req);
                res.totalHeadMovement += std::abs(req - currentHead);
                currentHead = req;
            }
        }
    } else {
        std::reverse(left.begin(), left.end());
        for (int32_t req : left) {
            res.serviceOrder.push_back(req);
            res.totalHeadMovement += std::abs(req - currentHead);
            currentHead = req;
        }
        if (!right.empty()) {
            res.totalHeadMovement += std::abs(0 - currentHead);
            currentHead = 0;

            for (int32_t req : right) {
                res.serviceOrder.push_back(req);
                res.totalHeadMovement += std::abs(req - currentHead);
                currentHead = req;
            }
        }
    }

    if (!requests.empty()) {
        res.averageHeadMovement = static_cast<double>(res.totalHeadMovement) / requests.size();
    }
    return res;
}

DiskScheduleResult DiskScheduler::scheduleCscan(int32_t initialHead, const std::vector<int32_t>& requests, int32_t maxCylinder) {
    DiskScheduleResult res;
    res.algorithm = DiskAlgorithm::CSCAN;
    res.algorithmName = "C-SCAN";
    res.initialHead = initialHead;
    res.maxCylinder = maxCylinder;

    std::vector<int32_t> left, right;
    for (int32_t req : requests) {
        if (req < initialHead) left.push_back(req);
        else right.push_back(req);
    }

    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    int32_t currentHead = initialHead;

    // Service right towards maxCylinder
    for (int32_t req : right) {
        res.serviceOrder.push_back(req);
        res.totalHeadMovement += std::abs(req - currentHead);
        currentHead = req;
    }

    if (!left.empty()) {
        res.totalHeadMovement += std::abs(maxCylinder - currentHead);
        // Circular jump to cylinder 0
        res.totalHeadMovement += maxCylinder; // 199 to 0
        currentHead = 0;

        for (int32_t req : left) {
            res.serviceOrder.push_back(req);
            res.totalHeadMovement += std::abs(req - currentHead);
            currentHead = req;
        }
    }

    if (!requests.empty()) {
        res.averageHeadMovement = static_cast<double>(res.totalHeadMovement) / requests.size();
    }
    return res;
}

std::string DiskScheduler::formatResultReport(const DiskScheduleResult& result) {
    std::ostringstream oss;
    oss << "Disk Scheduling Analysis [" << result.algorithmName << "]:\n";
    oss << "+--------------------------+-----------------------------------+\n";
    oss << "| Initial Head Position    | " << std::setw(33) << result.initialHead << " |\n";
    oss << "| Total Head Movement      | " << std::setw(33) << result.totalHeadMovement << " cylinders |\n";
    oss << "| Average Seek per Request | " << std::setw(33) << std::fixed << std::setprecision(2) << result.averageHeadMovement << " cylinders |\n";

    std::ostringstream order;
    for (size_t i = 0; i < result.serviceOrder.size(); ++i) {
        order << result.serviceOrder[i] << (i + 1 < result.serviceOrder.size() ? " -> " : "");
    }
    oss << "| Service Sequence         | " << std::left << std::setw(33) << order.str().substr(0, 33) << std::right << " |\n";
    oss << "+--------------------------+-----------------------------------+\n";
    return oss.str();
}

} // namespace io
} // namespace watchos
