#ifndef WATCHOS_PROCESS_LIFECYCLE_SIMULATOR_HPP
#define WATCHOS_PROCESS_LIFECYCLE_SIMULATOR_HPP

#include "watchos/core/types.hpp"
#include "watchos/process/process.hpp"
#include <vector>
#include <string>
#include <map>

namespace watchos {
namespace process {

struct StateTransition {
    core::TimeUnits timestamp = 0;
    core::ProcessState fromState;
    core::ProcessState toState;
    std::string reason;
};

struct ProcessLifecycleMetrics {
    core::ProcessId pid = 0;
    core::ProcessState currentState = core::ProcessState::NEW;
    core::TimeUnits arrivalTime = 0;
    core::TimeUnits firstCpuStartTime = -1;
    core::TimeUnits cpuTimeConsumed = 0;
    core::TimeUnits readyQueueWait = 0;
    core::TimeUnits ioWait = 0;
    core::TimeUnits completionTime = -1;
    std::string terminationReason;
    std::vector<StateTransition> history;

    core::TimeUnits lastReadyEntryTime = -1;
    core::TimeUnits lastWaitingEntryTime = -1;
};

class LifecycleSimulator {
public:
    explicit LifecycleSimulator(const std::vector<Process>& processes);

    bool transition(core::ProcessId pid, core::ProcessState targetState, core::TimeUnits currentTime, const std::string& reason);

    const ProcessLifecycleMetrics* getMetrics(core::ProcessId pid) const;
    ProcessLifecycleMetrics* getMetrics(core::ProcessId pid);

    const std::map<core::ProcessId, ProcessLifecycleMetrics>& getAllMetrics() const { return processMetrics_; }

    bool allTerminated() const;

    // Helper checking validity of transition according to PRD Section 6.3
    static bool isValidTransition(core::ProcessState from, core::ProcessState to);

private:
    std::map<core::ProcessId, ProcessLifecycleMetrics> processMetrics_;
};

} // namespace process
} // namespace watchos

#endif // WATCHOS_PROCESS_LIFECYCLE_SIMULATOR_HPP
