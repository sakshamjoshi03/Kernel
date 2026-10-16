#ifndef WATCHOS_PROCESS_SUBSYSTEM_CONFIGS_HPP
#define WATCHOS_PROCESS_SUBSYSTEM_CONFIGS_HPP

#include "watchos/core/types.hpp"
#include <string>
#include <vector>
#include <map>

namespace watchos {
namespace process {

// Memory requirement configuration for a process
struct MemoryConfig {
    bool isConfigured = false;
    core::MemoryUnits demandUnits = 0;
    std::vector<int32_t> pageReferences; // Optional reference string
};

// Single I/O operation request
struct IoRequest {
    core::TimeUnits requestTime = 0; // Relative CPU burst offset when I/O is requested
    core::TimeUnits duration = 1;     // I/O service duration
    std::string device = "DISK";      // Device name (e.g. DISK, NET)
};

// Resource claim and allocation for Deadlock / Banker's algorithms
struct ResourceClaim {
    std::string resourceType;
    int32_t maxClaim = 0;
    int32_t initialAllocation = 0;
};

// Disk head request for disk scheduling
struct DiskRequest {
    int32_t blockOrCylinder = 0;
    std::string operationType = "READ"; // READ or WRITE
    int64_t dataVolumeBytes = 4096;
};

// Synchronization requirement (mutex / semaphore / critical section)
struct SyncRequirement {
    std::string primitiveName;
    core::TimeUnits csStartTime = 0;
    core::TimeUnits csDuration = 1;
};

// Access control & permission configuration
struct AccessConfig {
    bool isConfigured = false;
    std::string userOrRole = "default_user";
    std::string resourceName;
    std::string requestedOperation = "READ"; // READ, WRITE, EXECUTE
};

} // namespace process
} // namespace watchos

#endif // WATCHOS_PROCESS_SUBSYSTEM_CONFIGS_HPP
