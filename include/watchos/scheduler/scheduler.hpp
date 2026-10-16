#ifndef WATCHOS_SCHEDULER_SCHEDULER_HPP
#define WATCHOS_SCHEDULER_SCHEDULER_HPP

#include "watchos/process/process.hpp"
#include "watchos/scheduler/scheduling_metrics.hpp"
#include <string>
#include <vector>
#include <memory>

namespace watchos {
namespace scheduler {

class Scheduler {
public:
    virtual ~Scheduler() = default;

    virtual std::string getName() const = 0;
    virtual bool isPreemptive() const = 0;

    virtual AggregateMetrics schedule(
        const std::vector<process::Process>& processes,
        const core::SystemSettings& settings
    ) = 0;
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_SCHEDULER_HPP
