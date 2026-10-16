#include "watchos/core/id_generator.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>
#include <atomic>

namespace watchos {
namespace core {



std::string IdGenerator::generateSessionId() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_ptr = std::localtime(&now_time);
    std::tm tm_buf = tm_ptr ? *tm_ptr : std::tm{};

    static std::atomic<uint32_t> sessionSeq{
        static_cast<uint32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xFFFF)
    };
    uint32_t seq = sessionSeq.fetch_add(1) & 0xFFFF;

    std::ostringstream oss;
    oss << "WOS-"
        << std::put_time(&tm_buf, "%Y%m%d")
        << "-"
        << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << seq;
    return oss.str();
}

std::string IdGenerator::generateRunId(uint32_t sequenceNumber) {
    if (sequenceNumber > 0) {
        std::ostringstream oss;
        oss << "RUN-" << std::setw(6) << std::setfill('0') << sequenceNumber;
        return oss.str();
    }
    static std::atomic<uint32_t> counter{1};
    uint32_t seq = counter.fetch_add(1);
    std::ostringstream oss;
    oss << "RUN-" << std::setw(6) << std::setfill('0') << seq;
    return oss.str();
}

std::string IdGenerator::getCurrentTimestampIso() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_ptr = std::gmtime(&now_time);
    std::tm tm_buf = tm_ptr ? *tm_ptr : std::tm{};

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S UTC");
    return oss.str();
}

} // namespace core
} // namespace watchos
