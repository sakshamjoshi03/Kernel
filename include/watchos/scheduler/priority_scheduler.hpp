#ifndef WATCHOS_SCHEDULER_PRIORITY_SCHEDULER_HPP
#define WATCHOS_SCHEDULER_PRIORITY_SCHEDULER_HPP

#include "watchos/scheduler/scheduler.hpp"

namespace watchos {
namespace scheduler {

class PriorityScheduler : public Scheduler {
public:
    explicit PriorityScheduler(bool preemptive = false, bool applyAging = false, core::TimeUnits agingInterval = 5, core::Priority agingIncrement = 1)
        : preemptive_(preemptive), applyAging_(applyAging), agingInterval_(agingInterval), agingIncrement_(agingIncrement) {}

    std::string getName() const override {
        std::string mode = preemptive_ ? "Priority (Preemptive)" : "Priority (Non-Preemptive)";
        if (applyAging_) mode += " with Aging";
        return mode;
    }

    bool isPreemptive() const override { return preemptive_; }

    AggregateMetrics schedule(
        const std::vector<process::Process>& processes,
        const core::SystemSettings& settings
    ) override;

private:
    bool preemptive_ = false;
    bool applyAging_ = false;
    core::TimeUnits agingInterval_ = 5;
    core::Priority agingIncrement_ = 1;
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_PRIORITY_SCHEDULER_HPP
