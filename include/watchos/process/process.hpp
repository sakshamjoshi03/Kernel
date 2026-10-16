#ifndef WATCHOS_PROCESS_PROCESS_HPP
#define WATCHOS_PROCESS_PROCESS_HPP

#include "watchos/core/types.hpp"
#include "watchos/process/subsystem_configs.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace process {

class Process {
public:
    Process() = default;
    Process(core::ProcessId pid, core::TimeUnits arrivalTime, core::TimeUnits cpuBurst, core::Priority priority);

    // Core fields
    core::ProcessId getPid() const { return pid_; }
    void setPid(core::ProcessId pid) { pid_ = pid; }

    core::TimeUnits getArrivalTime() const { return arrivalTime_; }
    void setArrivalTime(core::TimeUnits at) { arrivalTime_ = at; }

    core::TimeUnits getCpuBurst() const { return cpuBurst_; }
    void setCpuBurst(core::TimeUnits burst) { cpuBurst_ = burst; }

    core::Priority getPriority() const { return priority_; }
    void setPriority(core::Priority p) { priority_ = p; }

    // Subsystems
    const MemoryConfig& getMemoryConfig() const { return memoryConfig_; }
    MemoryConfig& getMemoryConfig() { return memoryConfig_; }
    void setMemoryConfig(const MemoryConfig& cfg) { memoryConfig_ = cfg; }

    const std::vector<IoRequest>& getIoRequests() const { return ioRequests_; }
    std::vector<IoRequest>& getIoRequests() { return ioRequests_; }
    void addIoRequest(const IoRequest& req) { ioRequests_.push_back(req); }
    void clearIoRequests() { ioRequests_.clear(); }

    const std::vector<ResourceClaim>& getResourceClaims() const { return resourceClaims_; }
    std::vector<ResourceClaim>& getResourceClaims() { return resourceClaims_; }
    void addResourceClaim(const ResourceClaim& claim) { resourceClaims_.push_back(claim); }
    void clearResourceClaims() { resourceClaims_.clear(); }

    const std::vector<DiskRequest>& getDiskRequests() const { return diskRequests_; }
    std::vector<DiskRequest>& getDiskRequests() { return diskRequests_; }
    void addDiskRequest(const DiskRequest& req) { diskRequests_.push_back(req); }
    void clearDiskRequests() { diskRequests_.clear(); }

    const std::vector<SyncRequirement>& getSyncRequirements() const { return syncRequirements_; }
    std::vector<SyncRequirement>& getSyncRequirements() { return syncRequirements_; }
    void addSyncRequirement(const SyncRequirement& sync) { syncRequirements_.push_back(sync); }
    void clearSyncRequirements() { syncRequirements_.clear(); }

    const AccessConfig& getAccessConfig() const { return accessConfig_; }
    AccessConfig& getAccessConfig() { return accessConfig_; }
    void setAccessConfig(const AccessConfig& cfg) { accessConfig_ = cfg; }

    // Summary representation for console display
    std::string getSummaryString() const;

private:
    core::ProcessId pid_ = 0;
    core::TimeUnits arrivalTime_ = 0;
    core::TimeUnits cpuBurst_ = 1;
    core::Priority priority_ = 1; // Lower number = higher priority

    MemoryConfig memoryConfig_;
    std::vector<IoRequest> ioRequests_;
    std::vector<ResourceClaim> resourceClaims_;
    std::vector<DiskRequest> diskRequests_;
    std::vector<SyncRequirement> syncRequirements_;
    AccessConfig accessConfig_;
};

} // namespace process
} // namespace watchos

#endif // WATCHOS_PROCESS_PROCESS_HPP
