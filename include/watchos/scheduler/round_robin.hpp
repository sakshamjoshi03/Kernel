#ifndef WATCHOS_SCHEDULER_ROUND_ROBIN_HPP
#define WATCHOS_SCHEDULER_ROUND_ROBIN_HPP

#include "watchos/scheduler/scheduler.hpp"

namespace watchos {
namespace scheduler {

class RoundRobinScheduler : public Scheduler {
public:
    explicit RoundRobinScheduler(core::TimeUnits quantum = 2) : timeQuantum_(quantum) {}

    std::string getName() const override {
        return "Round Robin (Q=" + std::to_string(timeQuantum_) + ")";
    }
    bool isPreemptive() const override { return true; }

    core::TimeUnits getTimeQuantum() const { return timeQuantum_; }
    void setTimeQuantum(core::TimeUnits q) { timeQuantum_ = q; }

    AggregateMetrics schedule(
        const std::vector<process::Process>& processes,
        const core::SystemSettings& settings
    ) override;

private:
    core::TimeUnits timeQuantum_ = 2;
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_ROUND_ROBIN_HPP
