#ifndef WATCHOS_PROCESS_WORKLOAD_VALIDATOR_HPP
#define WATCHOS_PROCESS_WORKLOAD_VALIDATOR_HPP

#include "watchos/process/process.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace process {

struct ValidationIssue {
    enum class Severity {
        WARNING,
        ERROR
    };

    Severity severity;
    core::ProcessId pid;
    std::string field;
    std::string message;
};

class WorkloadValidator {
public:
    // Validate an entire workload (list of processes)
    static std::vector<ValidationIssue> validateWorkload(const std::vector<Process>& processes);

    // Validate a single process in isolation or against existing processes
    static std::vector<ValidationIssue> validateProcess(const Process& proc, const std::vector<Process>& existing = {});

    // Check if the workload is valid for finalization (has at least 1 process and zero errors)
    static bool isWorkloadValid(const std::vector<Process>& processes, std::string& outErrorMessage);
};

} // namespace process
} // namespace watchos

#endif // WATCHOS_PROCESS_WORKLOAD_VALIDATOR_HPP
