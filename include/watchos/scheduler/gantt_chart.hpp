#ifndef WATCHOS_SCHEDULER_GANTT_CHART_HPP
#define WATCHOS_SCHEDULER_GANTT_CHART_HPP

#include "watchos/scheduler/scheduling_metrics.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace scheduler {

class GanttChart {
public:
    static std::string render(const std::string& title, const std::vector<GanttSegment>& segments);
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_GANTT_CHART_HPP
