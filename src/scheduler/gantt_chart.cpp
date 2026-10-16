#include "watchos/scheduler/gantt_chart.hpp"
#include <sstream>
#include <iomanip>

namespace watchos {
namespace scheduler {

std::string GanttChart::render(const std::string& title, const std::vector<GanttSegment>& segments) {
    if (segments.empty()) {
        return title + "\n(No execution segments recorded)\n";
    }

    std::ostringstream oss;
    oss << title << " Gantt Chart:\n";

    // Top border and process labels: | P1 | P2 | IDLE | P3 |
    std::ostringstream lineBar;
    std::ostringstream lineTime;

    lineBar << "|";
    lineTime << segments.front().startTime;

    for (const auto& seg : segments) {
        std::string label;
        if (seg.isIdle) {
            label = "IDLE";
        } else if (seg.isContextSwitch) {
            label = "CS";
        } else {
            label = "P" + std::to_string(seg.pid);
        }

        // Cell width based on label length + padding
        int width = std::max(static_cast<int>(label.length()) + 2, 6);

        // Center label in cell
        int padLeft = (width - static_cast<int>(label.length())) / 2;
        int padRight = width - static_cast<int>(label.length()) - padLeft;

        lineBar << std::string(padLeft, ' ') << label << std::string(padRight, ' ') << "|";

        // Space out time marker to line up under cell boundary
        std::string timeStr = std::to_string(seg.endTime);
        int timePadding = width + 1 - static_cast<int>(timeStr.length());
        if (timePadding > 0) {
            lineTime << std::string(timePadding, ' ') << timeStr;
        } else {
            lineTime << " " << timeStr;
        }
    }

    oss << lineBar.str() << "\n";
    oss << lineTime.str() << "\n\n";

    return oss.str();
}

} // namespace scheduler
} // namespace watchos
