#include "watchos/deadlock/resource_manager.hpp"
#include <sstream>
#include <iomanip>

namespace watchos {
namespace deadlock {

void ResourceManager::registerResourceType(const std::string& name, int32_t totalInstances) {
    ResourceTypeInfo info;
    info.name = name;
    info.totalInstances = totalInstances;
    info.availableInstances = totalInstances;
    resources_[name] = info;
}

void ResourceManager::setAllocation(core::ProcessId pid, const std::string& resName, int32_t count) {
    allocation_[pid][resName] = count;
}

int32_t ResourceManager::getAllocation(core::ProcessId pid, const std::string& resName) const {
    auto pIt = allocation_.find(pid);
    if (pIt != allocation_.end()) {
        auto rIt = pIt->second.find(resName);
        if (rIt != pIt->second.end()) return rIt->second;
    }
    return 0;
}

void ResourceManager::setRequest(core::ProcessId pid, const std::string& resName, int32_t count) {
    request_[pid][resName] = count;
}

int32_t ResourceManager::getRequest(core::ProcessId pid, const std::string& resName) const {
    auto pIt = request_.find(pid);
    if (pIt != request_.end()) {
        auto rIt = pIt->second.find(resName);
        if (rIt != pIt->second.end()) return rIt->second;
    }
    return 0;
}

void ResourceManager::setMaxClaim(core::ProcessId pid, const std::string& resName, int32_t count) {
    maxClaim_[pid][resName] = count;
}

int32_t ResourceManager::getMaxClaim(core::ProcessId pid, const std::string& resName) const {
    auto pIt = maxClaim_.find(pid);
    if (pIt != maxClaim_.end()) {
        auto rIt = pIt->second.find(resName);
        if (rIt != pIt->second.end()) return rIt->second;
    }
    return 0;
}

void ResourceManager::loadFromWorkload(const std::vector<process::Process>& processes) {
    resources_.clear();
    allocation_.clear();
    request_.clear();
    maxClaim_.clear();

    // Collect all declared resource types
    for (const auto& proc : processes) {
        for (const auto& claim : proc.getResourceClaims()) {
            if (resources_.find(claim.resourceType) == resources_.end()) {
                registerResourceType(claim.resourceType, 0);
            }
            setMaxClaim(proc.getPid(), claim.resourceType, claim.maxClaim);
            setAllocation(proc.getPid(), claim.resourceType, claim.initialAllocation);
        }
    }

    if (resources_.empty()) {
        // Default resources if workload has none configured
        registerResourceType("R1", 10);
        registerResourceType("R2", 5);
        registerResourceType("R3", 7);

        for (const auto& proc : processes) {
            setMaxClaim(proc.getPid(), "R1", 5);
            setMaxClaim(proc.getPid(), "R2", 2);
            setMaxClaim(proc.getPid(), "R3", 3);

            setAllocation(proc.getPid(), "R1", 1);
            setAllocation(proc.getPid(), "R2", 0);
            setAllocation(proc.getPid(), "R3", 1);
        }
    }

    // Compute total instances and available
    for (auto& rPair : resources_) {
        int32_t allocatedTotal = 0;
        int32_t maxClaimMax = 0;
        for (const auto& p : processes) {
            allocatedTotal += getAllocation(p.getPid(), rPair.first);
            maxClaimMax = std::max(maxClaimMax, getMaxClaim(p.getPid(), rPair.first));
        }
        if (rPair.second.totalInstances <= allocatedTotal) {
            rPair.second.totalInstances = allocatedTotal + 3; // reserve 3 available
        }
        rPair.second.availableInstances = rPair.second.totalInstances - allocatedTotal;
    }
}

std::string ResourceManager::renderResourceAllocationGraph() const {
    std::ostringstream oss;
    oss << "Resource Allocation Graph (RAG):\n";
    oss << "+-----------+-------------------------+-------------------------+\n";
    oss << "| Process   | Allocated Edges (R -> P)| Request Edges (P -> R)  |\n";
    oss << "+-----------+-------------------------+-------------------------+\n";

    for (const auto& pPair : maxClaim_) {
        core::ProcessId pid = pPair.first;
        std::ostringstream allocStream, reqStream;

        for (const auto& rPair : resources_) {
            int allocCount = getAllocation(pid, rPair.first);
            if (allocCount > 0) {
                allocStream << rPair.first << "(" << allocCount << ") ";
            }
            int reqCount = getRequest(pid, rPair.first);
            if (reqCount > 0) {
                reqStream << rPair.first << "(" << reqCount << ") ";
            }
        }

        std::string allocStr = allocStream.str().empty() ? "(none)" : allocStream.str();
        std::string reqStr = reqStream.str().empty() ? "(none)" : reqStream.str();

        oss << "| PID " << std::setw(5) << pid
            << " | " << std::left << std::setw(23) << allocStr << std::right
            << " | " << std::left << std::setw(23) << reqStr << std::right << " |\n";
    }
    oss << "+-----------+-------------------------+-------------------------+\n";

    oss << "\nResource Availability:\n";
    for (const auto& rPair : resources_) {
        oss << "  " << rPair.first << ": Available " << rPair.second.availableInstances
            << " of " << rPair.second.totalInstances << " total instances.\n";
    }

    return oss.str();
}

} // namespace deadlock
} // namespace watchos
