#include "watchos/process/workload_validator.hpp"
#include <set>
#include <sstream>

namespace watchos {
namespace process {

std::vector<ValidationIssue> WorkloadValidator::validateProcess(const Process& proc, const std::vector<Process>& existing) {
    std::vector<ValidationIssue> issues;

    // PID validation
    if (proc.getPid() <= 0) {
        issues.push_back({
            ValidationIssue::Severity::ERROR,
            proc.getPid(),
            "PID",
            "Process ID must be a positive integer (> 0)."
        });
    }

    // Check duplicate PID
    for (const auto& other : existing) {
        if (other.getPid() == proc.getPid()) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "PID",
                "Duplicate PID detected. Process ID must be unique within the session."
            });
            break;
        }
    }

    // Arrival time validation: Non-negative integer
    if (proc.getArrivalTime() < 0) {
        issues.push_back({
            ValidationIssue::Severity::ERROR,
            proc.getPid(),
            "Arrival Time",
            "Arrival time cannot be negative."
        });
    }

    // CPU burst validation: Strictly positive integer
    if (proc.getCpuBurst() <= 0) {
        issues.push_back({
            ValidationIssue::Severity::ERROR,
            proc.getPid(),
            "CPU Burst",
            "CPU burst time must be a strictly positive integer (> 0)."
        });
    }

    // Memory demand validation (if configured)
    if (proc.getMemoryConfig().isConfigured) {
        if (proc.getMemoryConfig().demandUnits <= 0) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "Memory Demand",
                "Configured memory demand must be greater than 0."
            });
        }
    }

    // I/O requests validation
    for (size_t i = 0; i < proc.getIoRequests().size(); ++i) {
        const auto& io = proc.getIoRequests()[i];
        if (io.requestTime < 0) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "I/O Request Time",
                "I/O request time cannot be negative."
            });
        }
        if (io.requestTime >= proc.getCpuBurst()) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "I/O Request Time",
                "I/O request time (" + std::to_string(io.requestTime) + 
                ") must be strictly less than CPU burst (" + std::to_string(proc.getCpuBurst()) + ")."
            });
        }
        if (io.duration <= 0) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "I/O Duration",
                "I/O duration must be a positive integer (> 0)."
            });
        }
    }

    // Resource claims validation
    for (const auto& rc : proc.getResourceClaims()) {
        if (rc.maxClaim < 0) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "Resource Claim",
                "Max claim for resource '" + rc.resourceType + "' cannot be negative."
            });
        }
        if (rc.initialAllocation < 0) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "Resource Allocation",
                "Initial allocation for resource '" + rc.resourceType + "' cannot be negative."
            });
        }
        if (rc.initialAllocation > rc.maxClaim) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "Resource Claim vs Allocation",
                "Initial allocation (" + std::to_string(rc.initialAllocation) +
                ") cannot exceed maximum claim (" + std::to_string(rc.maxClaim) + ") for resource '" + rc.resourceType + "'."
            });
        }
    }

    // Disk requests validation
    for (const auto& dr : proc.getDiskRequests()) {
        if (dr.blockOrCylinder < 0) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "Disk Cylinder/Block",
                "Disk cylinder/block address cannot be negative."
            });
        }
    }

    // Synchronization validation
    for (const auto& sync : proc.getSyncRequirements()) {
        if (sync.csStartTime < 0) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "Critical Section Start",
                "Critical section start time cannot be negative."
            });
        }
        if (sync.csDuration <= 0) {
            issues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "Critical Section Duration",
                "Critical section duration must be positive."
            });
        }
    }

    return issues;
}

std::vector<ValidationIssue> WorkloadValidator::validateWorkload(const std::vector<Process>& processes) {
    std::vector<ValidationIssue> allIssues;

    if (processes.empty()) {
        allIssues.push_back({
            ValidationIssue::Severity::ERROR,
            0,
            "Workload",
            "At least one process must be configured in the session."
        });
        return allIssues;
    }

    std::set<core::ProcessId> seenPids;
    for (const auto& proc : processes) {
        if (seenPids.count(proc.getPid()) > 0) {
            allIssues.push_back({
                ValidationIssue::Severity::ERROR,
                proc.getPid(),
                "PID",
                "Duplicate process ID " + std::to_string(proc.getPid()) + " found in workload."
            });
        }
        seenPids.insert(proc.getPid());

        // Validate individual process
        auto singleIssues = validateProcess(proc);
        allIssues.insert(allIssues.end(), singleIssues.begin(), singleIssues.end());
    }

    return allIssues;
}

bool WorkloadValidator::isWorkloadValid(const std::vector<Process>& processes, std::string& outErrorMessage) {
    auto issues = validateWorkload(processes);
    for (const auto& issue : issues) {
        if (issue.severity == ValidationIssue::Severity::ERROR) {
            outErrorMessage = "[" + issue.field + "] " + issue.message;
            return false;
        }
    }
    return true;
}

} // namespace process
} // namespace watchos
