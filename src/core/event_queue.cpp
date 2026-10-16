#include "watchos/core/event_queue.hpp"

namespace watchos {
namespace core {

void EventQueue::push(TimeUnits timestamp, EventType type, ProcessId entityId, const std::string& desc) {
    Event ev;
    ev.eventId = nextEventId_++;
    ev.timestamp = timestamp;
    ev.type = type;
    ev.entityId = entityId;
    ev.description = desc;
    queue_.push(ev);
}

void EventQueue::push(const Event& event) {
    Event ev = event;
    if (ev.eventId == 0) {
        ev.eventId = nextEventId_++;
    }
    queue_.push(ev);
}

bool EventQueue::empty() const {
    return queue_.empty();
}

size_t EventQueue::size() const {
    return queue_.size();
}

Event EventQueue::pop() {
    Event topEv = queue_.top();
    queue_.pop();
    return topEv;
}

const Event& EventQueue::top() const {
    return queue_.top();
}

void EventQueue::clear() {
    while (!queue_.empty()) {
        queue_.pop();
    }
    nextEventId_ = 1;
}

} // namespace core
} // namespace watchos
