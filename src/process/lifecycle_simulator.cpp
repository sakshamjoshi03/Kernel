#include "watchos/process/lifecycle_simulator.hpp"
#include <iostream>

namespace watchos {
namespace process {

LifecycleSimulator::LifecycleSimulator(const std::vector<Process>& processes) {
    for (const auto& p : processes) {
        ProcessLifecycleMetrics metrics;
        metrics.pid = p.getPid();
        metrics.currentState = core::ProcessState::NEW;
        metrics.arrivalTime = p.getArrivalTime();
        processMetrics_[p.getPid()] = metrics;
    }
}

bool LifecycleSimulator::isValidTransition(core::ProcessState from, core::ProcessState to) {
    // Required transitions from PRD Section 6.3:
    // NEW -> READY
    // READY -> RUNNING
    // RUNNING -> READY
    // RUNNING -> WAITING
    // WAITING -> READY
    // RUNNING -> TERMINATED
    if (from == core::ProcessState::NEW && to == core::ProcessState::READY) return true;
    if (from == core::ProcessState::READY && to == core::ProcessState::RUNNING) return true;
    if (from == core::ProcessState::RUNNING && to == core::ProcessState::READY) return true;
    if (from == core::ProcessState::RUNNING && to == core::ProcessState::WAITING) return true;
    if (from == core::ProcessState::WAITING && to == core::ProcessState::READY) return true;
    if (from == core::ProcessState::RUNNING && to == core::ProcessState::TERMINATED) return true;
    return false;
}

bool LifecycleSimulator::transition(core::ProcessId pid, core::ProcessState targetState, core::TimeUnits currentTime, const std::string& reason) {
    auto it = processMetrics_.find(pid);
    if (it == processMetrics_.end()) {
        std::cerr << "[ERROR] Invalid transition attempted for unknown PID " << pid << "\n";
        return false;
    }

    auto& metrics = it->second;
    if (!isValidTransition(metrics.currentState, targetState)) {
        std::cerr << "[ERROR] Illegal state transition for PID " << pid
                  << " from " << core::processStateToString(metrics.currentState)
                  << " to " << core::processStateToString(targetState)
                  << " (Reason: " << reason << ")\n";
        return false;
    }

    // Accumulate time spent in previous state
    if (metrics.currentState == core::ProcessState::READY) {
        if (metrics.lastReadyEntryTime >= 0 && currentTime >= metrics.lastReadyEntryTime) {
            metrics.readyQueueWait += (currentTime - metrics.lastReadyEntryTime);
        }
    } else if (metrics.currentState == core::ProcessState::WAITING) {
        if (metrics.lastWaitingEntryTime >= 0 && currentTime >= metrics.lastWaitingEntryTime) {
            metrics.ioWait += (currentTime - metrics.lastWaitingEntryTime);
        }
    }

    // Set markers for entering new state
    if (targetState == core::ProcessState::READY) {
        metrics.lastReadyEntryTime = currentTime;
    } else if (targetState == core::ProcessState::WAITING) {
        metrics.lastWaitingEntryTime = currentTime;
    } else if (targetState == core::ProcessState::RUNNING) {
        if (metrics.firstCpuStartTime < 0) {
            metrics.firstCpuStartTime = currentTime;
        }
    } else if (targetState == core::ProcessState::TERMINATED) {
        metrics.completionTime = currentTime;
        metrics.terminationReason = reason.empty() ? "Execution completed" : reason;
    }

    StateTransition trans;
    trans.timestamp = currentTime;
    trans.fromState = metrics.currentState;
    trans.toState = targetState;
    trans.reason = reason;
    metrics.history.push_back(trans);

    metrics.currentState = targetState;
    return true;
}

const ProcessLifecycleMetrics* LifecycleSimulator::getMetrics(core::ProcessId pid) const {
    auto it = processMetrics_.find(pid);
    return (it != processMetrics_.end()) ? &it->second : nullptr;
}

ProcessLifecycleMetrics* LifecycleSimulator::getMetrics(core::ProcessId pid) {
    auto it = processMetrics_.find(pid);
    return (it != processMetrics_.end()) ? &it->second : nullptr;
}

bool LifecycleSimulator::allTerminated() const {
    for (const auto& pair : processMetrics_) {
        if (pair.second.currentState != core::ProcessState::TERMINATED) {
            return false;
        }
    }
    return !processMetrics_.empty();
}

} // namespace process
} // namespace watchos
