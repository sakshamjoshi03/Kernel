#ifndef WATCHOS_CORE_TYPES_HPP
#define WATCHOS_CORE_TYPES_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace watchos {
namespace core {

using ProcessId = int32_t;
using TimeUnits = int64_t;
using MemoryUnits = int64_t;
using Priority = int32_t;

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

inline std::string processStateToString(ProcessState state) {
    switch (state) {
        case ProcessState::NEW: return "NEW";
        case ProcessState::READY: return "READY";
        case ProcessState::RUNNING: return "RUNNING";
        case ProcessState::WAITING: return "WAITING";
        case ProcessState::TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

enum class SessionStatus {
    DRAFT,
    FINALIZED,
    ARCHIVED
};

inline std::string sessionStatusToString(SessionStatus status) {
    switch (status) {
        case SessionStatus::DRAFT: return "DRAFT";
        case SessionStatus::FINALIZED: return "FINALIZED";
        case SessionStatus::ARCHIVED: return "ARCHIVED";
        default: return "UNKNOWN";
    }
}

inline SessionStatus stringToSessionStatus(const std::string& str) {
    if (str == "FINALIZED") return SessionStatus::FINALIZED;
    if (str == "ARCHIVED") return SessionStatus::ARCHIVED;
    return SessionStatus::DRAFT;
}

// System configuration settings
struct SystemSettings {
    TimeUnits defaultTimeQuantum = 2;
    int32_t defaultMemoryFrames = 16;
    TimeUnits agingInterval = 5;
    Priority agingIncrement = 1;
    TimeUnits contextSwitchOverhead = 0;
    bool enableAnsiColor = true;
    std::string reportFormat = "MARKDOWN";
};

} // namespace core
} // namespace watchos

#endif // WATCHOS_CORE_TYPES_HPP
