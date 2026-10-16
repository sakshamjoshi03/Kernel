#include "watchos/io/io_manager.hpp"
#include <sstream>
#include <iomanip>

namespace watchos {
namespace io {

void IoManager::enqueueOperation(core::ProcessId pid, const process::IoRequest& req, core::TimeUnits currentTime) {
    QueuedIoOperation op;
    op.pid = pid;
    op.requestTime = req.requestTime;
    op.duration = req.duration;
    op.device = req.device;
    op.queueEntryTime = currentTime;

    deviceQueues_[req.device].push_back(op);
}

std::vector<core::ProcessId> IoManager::advanceSimulation(core::TimeUnits currentTime) {
    std::vector<core::ProcessId> awakenedPids;

    for (auto& pair : deviceQueues_) {
        const std::string& dev = pair.first;
        auto& queue = pair.second;

        while (!queue.empty()) {
            auto& frontOp = queue.front();
            core::TimeUnits busyUntil = deviceBusyUntil_[dev];
            core::TimeUnits start = std::max(frontOp.queueEntryTime, busyUntil);
            core::TimeUnits finish = start + frontOp.duration;

            if (finish <= currentTime) {
                frontOp.serviceStartTime = start;
                frontOp.completionTime = finish;
                deviceBusyUntil_[dev] = finish;
                awakenedPids.push_back(frontOp.pid);
                completedOperations_.push_back(frontOp);
                queue.pop_front();
            } else {
                break; // Device busy or not finished yet
            }
        }
    }

    return awakenedPids;
}

std::string IoManager::formatSummary() const {
    std::ostringstream oss;
    oss << "I/O Management Operations Summary:\n";
    oss << "+-----+--------+------------+------------+---------------+------------+\n";
    oss << "| PID | Device | Queue Time | Start Time | Service Dur   | Finish     |\n";
    oss << "+-----+--------+------------+------------+---------------+------------+\n";

    if (completedOperations_.empty()) {
        oss << "|                 No I/O operations recorded.                         |\n";
    } else {
        for (const auto& op : completedOperations_) {
            oss << "| " << std::setw(3) << op.pid
                << " | " << std::left << std::setw(6) << op.device << std::right
                << " | " << std::setw(10) << op.queueEntryTime
                << " | " << std::setw(10) << op.serviceStartTime
                << " | " << std::setw(13) << op.duration
                << " | " << std::setw(10) << op.completionTime << " |\n";
        }
    }
    oss << "+-----+--------+------------+------------+---------------+------------+\n";
    return oss.str();
}

} // namespace io
} // namespace watchos
