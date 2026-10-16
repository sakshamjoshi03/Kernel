#include "watchos/experiments/fault_injector.hpp"
#include <sstream>

namespace watchos {
namespace experiments {

FaultRecord FaultInjector::injectFault(
    FaultType type,
    core::ProcessId targetPid,
    core::TimeUnits simulatedTime
) {
    FaultRecord rec;
    rec.timestamp = simulatedTime;
    rec.type = type;
    rec.targetPid = targetPid;

    switch (type) {
        case FaultType::DELAYED_IO:
            rec.faultDescription = "I/O subsystem hardware latency simulation: injected +10ms transfer latency.";
            rec.stateTransition = "Process PID " + std::to_string(targetPid) + " kept in WAITING queue longer.";
            rec.recoveryAction = "I/O Controller rescheduled completion timer at T=" + std::to_string(simulatedTime + 10);
            rec.finalOutcome = "Process completed I/O successfully after simulated delay without data corruption.";
            break;
        case FaultType::PROCESS_CRASH:
            rec.faultDescription = "Simulated abnormal process termination (SIGSEGV / unhandled exception).";
            rec.stateTransition = "RUNNING -> TERMINATED (Forced)";
            rec.recoveryAction = "Kernel reclaimed allocated memory partitions and released held resource locks.";
            rec.finalOutcome = "System integrity preserved. Process removed from scheduling run queue.";
            break;
        case FaultType::RESOURCE_SHORTAGE:
            rec.faultDescription = "Simulated exhaustion of shared system semaphore/lock instances.";
            rec.stateTransition = "READY -> WAITING (Resource starvation block)";
            rec.recoveryAction = "Deadlock prevention policy invoked: priority donation / resource revocation.";
            rec.finalOutcome = "Resource reclaimed and granted; starvation resolved.";
            break;
        case FaultType::MEMORY_PRESSURE:
            rec.faultDescription = "Simulated working set page fault storm (thrashing threshold breached).";
            rec.stateTransition = "Frequent page swapping detected.";
            rec.recoveryAction = "Working-set clock algorithm swapped lower-priority process pages to secondary backing store.";
            rec.finalOutcome = "Free frame pool restored; system thrashing alleviated.";
            break;
        case FaultType::DEVICE_UNAVAILABLE:
            rec.faultDescription = "Target device controller went offline / timed out.";
            rec.stateTransition = "I/O request blocked on controller ready-bit.";
            rec.recoveryAction = "I/O subsystem re-routed request to mirrored backup drive channel.";
            rec.finalOutcome = "Request fulfilled via fallback controller channel.";
            break;
    }

    return rec;
}

std::string FaultInjector::formatFaultReport(const FaultRecord& record) {
    std::ostringstream oss;
    oss << "Controlled Fault Injection Report:\n";
    oss << "  Simulated Time    : " << record.timestamp << " time units\n";
    oss << "  Fault Type        : " << faultTypeToString(record.type) << "\n";
    oss << "  Target Process    : PID " << record.targetPid << "\n";
    oss << "  Description       : " << record.faultDescription << "\n";
    oss << "  State Transition  : " << record.stateTransition << "\n";
    oss << "  Recovery Action   : " << record.recoveryAction << "\n";
    oss << "  Final Outcome     : " << record.finalOutcome << "\n";
    return oss.str();
}

} // namespace experiments
} // namespace watchos
