#ifndef WATCHOS_EXPERIMENTS_FAULT_INJECTOR_HPP
#define WATCHOS_EXPERIMENTS_FAULT_INJECTOR_HPP

#include "watchos/core/types.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace experiments {

enum class FaultType {
    DELAYED_IO,
    PROCESS_CRASH,
    RESOURCE_SHORTAGE,
    MEMORY_PRESSURE,
    DEVICE_UNAVAILABLE
};

inline std::string faultTypeToString(FaultType type) {
    switch (type) {
        case FaultType::DELAYED_IO: return "DELAYED_IO";
        case FaultType::PROCESS_CRASH: return "PROCESS_CRASH";
        case FaultType::RESOURCE_SHORTAGE: return "RESOURCE_SHORTAGE";
        case FaultType::MEMORY_PRESSURE: return "MEMORY_PRESSURE";
        case FaultType::DEVICE_UNAVAILABLE: return "DEVICE_UNAVAILABLE";
        default: return "UNKNOWN";
    }
}

struct FaultRecord {
    core::TimeUnits timestamp = 0;
    FaultType type;
    core::ProcessId targetPid = 0;
    std::string faultDescription;
    std::string stateTransition;
    std::string recoveryAction;
    std::string finalOutcome;
};

class FaultInjector {
public:
    static FaultRecord injectFault(
        FaultType type,
        core::ProcessId targetPid,
        core::TimeUnits simulatedTime
    );

    static std::string formatFaultReport(const FaultRecord& record);
};

} // namespace experiments
} // namespace watchos

#endif // WATCHOS_EXPERIMENTS_FAULT_INJECTOR_HPP
