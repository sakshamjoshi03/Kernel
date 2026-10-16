#ifndef WATCHOS_IO_IO_MANAGER_HPP
#define WATCHOS_IO_IO_MANAGER_HPP

#include "watchos/core/types.hpp"
#include "watchos/process/subsystem_configs.hpp"
#include <vector>
#include <string>
#include <map>
#include <deque>

namespace watchos {
namespace io {

struct QueuedIoOperation {
    core::ProcessId pid = 0;
    core::TimeUnits requestTime = 0;
    core::TimeUnits duration = 1;
    std::string device = "DISK";
    core::TimeUnits queueEntryTime = 0;
    core::TimeUnits serviceStartTime = 0;
    core::TimeUnits completionTime = 0;
};

class IoManager {
public:
    explicit IoManager() = default;

    void enqueueOperation(core::ProcessId pid, const process::IoRequest& req, core::TimeUnits currentTime);

    // Process device queue up to currentTime
    std::vector<core::ProcessId> advanceSimulation(core::TimeUnits currentTime);

    const std::map<std::string, std::deque<QueuedIoOperation>>& getDeviceQueues() const { return deviceQueues_; }
    const std::vector<QueuedIoOperation>& getCompletedOperations() const { return completedOperations_; }

    std::string formatSummary() const;

private:
    std::map<std::string, std::deque<QueuedIoOperation>> deviceQueues_;
    std::map<std::string, core::TimeUnits> deviceBusyUntil_;
    std::vector<QueuedIoOperation> completedOperations_;
};

} // namespace io
} // namespace watchos

#endif // WATCHOS_IO_IO_MANAGER_HPP
