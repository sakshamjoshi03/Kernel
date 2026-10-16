#ifndef WATCHOS_SCHEDULER_FCFS_HPP
#define WATCHOS_SCHEDULER_FCFS_HPP

#include "watchos/scheduler/scheduler.hpp"

namespace watchos {
namespace scheduler {

class FcfsScheduler : public Scheduler {
public:
    std::string getName() const override { return "FCFS"; }
    bool isPreemptive() const override { return false; }

    AggregateMetrics schedule(
        const std::vector<process::Process>& processes,
        const core::SystemSettings& settings
    ) override;
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_FCFS_HPP
