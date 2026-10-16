#ifndef WATCHOS_CORE_SIMULATION_ENGINE_HPP
#define WATCHOS_CORE_SIMULATION_ENGINE_HPP

#include "watchos/core/types.hpp"
#include "watchos/core/event_queue.hpp"
#include "watchos/core/run_context.hpp"
#include "watchos/database/run_repository.hpp"
#include <functional>
#include <vector>

namespace watchos {
namespace core {

using EventHandler = std::function<void(const Event&, TimeUnits)>;

class SimulationEngine {
public:
    explicit SimulationEngine(RunContext context, database::RunRepository* repo = nullptr);

    TimeUnits getCurrentTime() const { return currentTime_; }
    void setClock(TimeUnits t) { currentTime_ = t; }

    EventQueue& getEventQueue() { return eventQueue_; }
    const EventQueue& getEventQueue() const { return eventQueue_; }

    RunContext& getContext() { return context_; }
    const RunContext& getContext() const { return context_; }

    void registerHandler(EventType type, EventHandler handler);
    void scheduleEvent(TimeUnits time, EventType type, ProcessId entityId, const std::string& desc = "");

    // Step a single event forward
    bool step();

    // Run until event queue is empty or maxTime reached
    void runUntilEmpty(TimeUnits maxTime = 1000000);

    // Reset simulation engine state
    void reset();

    void recordEvent(const std::string& eventType, ProcessId entityId, const std::string& details);

private:
    TimeUnits currentTime_ = 0;
    EventQueue eventQueue_;
    RunContext context_;
    database::RunRepository* repo_ = nullptr;
    std::map<EventType, std::vector<EventHandler>> handlers_;
};

} // namespace core
} // namespace watchos

#endif // WATCHOS_CORE_SIMULATION_ENGINE_HPP
