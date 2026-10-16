#include "watchos/process/process.hpp"
#include <sstream>

namespace watchos {
namespace process {

Process::Process(core::ProcessId pid, core::TimeUnits arrivalTime, core::TimeUnits cpuBurst, core::Priority priority)
    : pid_(pid), arrivalTime_(arrivalTime), cpuBurst_(cpuBurst), priority_(priority) {}

std::string Process::getSummaryString() const {
    std::ostringstream oss;
    oss << "PID: " << pid_
        << " | AT: " << arrivalTime_
        << " | BT: " << cpuBurst_
        << " | Pri: " << priority_;
    
    if (memoryConfig_.isConfigured) {
        oss << " | Mem: " << memoryConfig_.demandUnits << " units";
    }
    if (!ioRequests_.empty()) {
        oss << " | I/O Ops: " << ioRequests_.size();
    }
    if (!resourceClaims_.empty()) {
        oss << " | Resources: " << resourceClaims_.size();
    }
    if (!diskRequests_.empty()) {
        oss << " | Disk Ops: " << diskRequests_.size();
    }
    return oss.str();
}

} // namespace process
} // namespace watchos
