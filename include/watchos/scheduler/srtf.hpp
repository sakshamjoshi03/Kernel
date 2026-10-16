#ifndef WATCHOS_SCHEDULER_SRTF_HPP
#define WATCHOS_SCHEDULER_SRTF_HPP

#include "watchos/scheduler/scheduler.hpp"

namespace watchos {
namespace scheduler {

class SrtfScheduler : public Scheduler {
public:
    std::string getName() const override { return "SRTF"; }
    bool isPreemptive() const override { return true; }

    AggregateMetrics schedule(
        const std::vector<process::Process>& processes,
        const core::SystemSettings& settings
    ) override;
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_SRTF_HPP
