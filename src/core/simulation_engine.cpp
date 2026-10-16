#include "watchos/core/simulation_engine.hpp"
#include <iostream>

namespace watchos {
namespace core {

SimulationEngine::SimulationEngine(RunContext context, database::RunRepository* repo)
    : context_(std::move(context)), repo_(repo) {}

void SimulationEngine::registerHandler(EventType type, EventHandler handler) {
    handlers_[type].push_back(std::move(handler));
}

void SimulationEngine::scheduleEvent(TimeUnits time, EventType type, ProcessId entityId, const std::string& desc) {
    eventQueue_.push(time, type, entityId, desc);
}

bool SimulationEngine::step() {
    if (eventQueue_.empty()) {
        return false;
    }

    Event ev = eventQueue_.pop();
    if (ev.timestamp > currentTime_) {
        currentTime_ = ev.timestamp;
    }
    context_.setSimulatedClock(currentTime_);

    // Dispatch to registered handlers
    auto it = handlers_.find(ev.type);
    if (it != handlers_.end()) {
        for (const auto& handler : it->second) {
            handler(ev, currentTime_);
        }
    }

    // Persist event if repository is attached
    if (repo_) {
        repo_->recordSimulationEvent(
            context_.getRunId(),
            currentTime_,
            eventTypeToString(ev.type),
            ev.entityId,
            ev.description
        );
    }

    return true;
}

void SimulationEngine::runUntilEmpty(TimeUnits maxTime) {
    while (!eventQueue_.empty() && currentTime_ <= maxTime) {
        step();
    }
}

void SimulationEngine::reset() {
    currentTime_ = 0;
    eventQueue_.clear();
    context_.setSimulatedClock(0);
}

void SimulationEngine::recordEvent(const std::string& eventType, ProcessId entityId, const std::string& details) {
    if (repo_) {
        repo_->recordSimulationEvent(
            context_.getRunId(),
            currentTime_,
            eventType,
            entityId,
            details
        );
    }
}

} // namespace core
} // namespace watchos
