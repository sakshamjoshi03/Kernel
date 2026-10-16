#ifndef WATCHOS_CORE_EVENT_HPP
#define WATCHOS_CORE_EVENT_HPP

#include "watchos/core/types.hpp"
#include <string>
#include <functional>

namespace watchos {
namespace core {

enum class EventType {
    OP_COMPLETION,       // CPU burst slice or operation finished
    IO_COMPLETION,       // I/O operation finished -> move WAITING to READY
    PROCESS_ARRIVAL,     // Process enters system -> move NEW to READY
    RESOURCE_EVENT,      // Resource requested or released
    QUANTUM_EXPIRATION,  // Round Robin quantum expired -> preempt RUNNING to READY
    SCHEDULING_DECISION, // Scheduler selects next process
    FAULT_INJECTION,     // Injected simulated fault
    CUSTOM
};

inline int getEventTypePriority(EventType type) {
    // Lower integer = processed first when timestamps are equal (PRD Section 6.2)
    switch (type) {
        case EventType::OP_COMPLETION: return 10;
        case EventType::IO_COMPLETION: return 20;
        case EventType::PROCESS_ARRIVAL: return 30;
        case EventType::RESOURCE_EVENT: return 40;
        case EventType::QUANTUM_EXPIRATION: return 50;
        case EventType::SCHEDULING_DECISION: return 60;
        case EventType::FAULT_INJECTION: return 70;
        case EventType::CUSTOM: return 80;
        default: return 100;
    }
}

inline std::string eventTypeToString(EventType type) {
    switch (type) {
        case EventType::OP_COMPLETION: return "OP_COMPLETION";
        case EventType::IO_COMPLETION: return "IO_COMPLETION";
        case EventType::PROCESS_ARRIVAL: return "PROCESS_ARRIVAL";
        case EventType::RESOURCE_EVENT: return "RESOURCE_EVENT";
        case EventType::QUANTUM_EXPIRATION: return "QUANTUM_EXPIRATION";
        case EventType::SCHEDULING_DECISION: return "SCHEDULING_DECISION";
        case EventType::FAULT_INJECTION: return "FAULT_INJECTION";
        case EventType::CUSTOM: return "CUSTOM";
        default: return "UNKNOWN";
    }
}

struct Event {
    int64_t eventId = 0;
    TimeUnits timestamp = 0;
    EventType type = EventType::CUSTOM;
    ProcessId entityId = 0; // PID or Resource ID
    std::string description;

    // Strict deterministic comparator:
    // 1. Timestamp (ascending)
    // 2. Event type priority (lower int first)
    // 3. Entity ID (ascending)
    // 4. Unique eventId tie breaker (ascending)
    bool operator>(const Event& other) const {
        if (timestamp != other.timestamp) {
            return timestamp > other.timestamp;
        }
        int p1 = getEventTypePriority(type);
        int p2 = getEventTypePriority(other.type);
        if (p1 != p2) {
            return p1 > p2;
        }
        if (entityId != other.entityId) {
            return entityId > other.entityId;
        }
        return eventId > other.eventId;
    }
};

} // namespace core
} // namespace watchos

#endif // WATCHOS_CORE_EVENT_HPP
