#ifndef WATCHOS_DEADLOCK_RESOURCE_MANAGER_HPP
#define WATCHOS_DEADLOCK_RESOURCE_MANAGER_HPP

#include "watchos/core/types.hpp"
#include "watchos/process/process.hpp"
#include <string>
#include <vector>
#include <map>

namespace watchos {
namespace deadlock {

struct ResourceTypeInfo {
    std::string name;
    int32_t totalInstances = 1;
    int32_t availableInstances = 1;
};

class ResourceManager {
public:
    ResourceManager() = default;

    void registerResourceType(const std::string& name, int32_t totalInstances);

    const std::map<std::string, ResourceTypeInfo>& getResourceTypes() const { return resources_; }
    std::map<std::string, ResourceTypeInfo>& getResourceTypes() { return resources_; }

    void setAllocation(core::ProcessId pid, const std::string& resName, int32_t count);
    int32_t getAllocation(core::ProcessId pid, const std::string& resName) const;

    void setRequest(core::ProcessId pid, const std::string& resName, int32_t count);
    int32_t getRequest(core::ProcessId pid, const std::string& resName) const;

    void setMaxClaim(core::ProcessId pid, const std::string& resName, int32_t count);
    int32_t getMaxClaim(core::ProcessId pid, const std::string& resName) const;

    // Load from configured process list
    void loadFromWorkload(const std::vector<process::Process>& processes);

    std::string renderResourceAllocationGraph() const;

private:
    std::map<std::string, ResourceTypeInfo> resources_;
    std::map<core::ProcessId, std::map<std::string, int32_t>> allocation_;
    std::map<core::ProcessId, std::map<std::string, int32_t>> request_;
    std::map<core::ProcessId, std::map<std::string, int32_t>> maxClaim_;
};

} // namespace deadlock
} // namespace watchos

#endif // WATCHOS_DEADLOCK_RESOURCE_MANAGER_HPP
