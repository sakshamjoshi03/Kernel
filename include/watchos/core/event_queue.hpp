#ifndef WATCHOS_CORE_EVENT_QUEUE_HPP
#define WATCHOS_CORE_EVENT_QUEUE_HPP

#include "watchos/core/event.hpp"
#include <queue>
#include <vector>
#include <atomic>

namespace watchos {
namespace core {

class EventQueue {
public:
    EventQueue() = default;

    void push(TimeUnits timestamp, EventType type, ProcessId entityId, const std::string& desc = "");
    void push(const Event& event);

    bool empty() const;
    size_t size() const;
    Event pop();
    const Event& top() const;
    void clear();

private:
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> queue_;
    int64_t nextEventId_ = 1;
};

} // namespace core
} // namespace watchos

#endif // WATCHOS_CORE_EVENT_QUEUE_HPP
