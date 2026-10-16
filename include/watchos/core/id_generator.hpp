#ifndef WATCHOS_CORE_ID_GENERATOR_HPP
#define WATCHOS_CORE_ID_GENERATOR_HPP

#include <string>

namespace watchos {
namespace core {

class IdGenerator {
public:
    // Generate collision-resistant human-readable Session ID: WOS-YYYYMMDD-XXXX
    static std::string generateSessionId();

    // Generate unique Run ID: RUN-XXXXXX
    static std::string generateRunId(uint32_t sequenceNumber = 0);

    // Current ISO-8601 UTC timestamp string
    static std::string getCurrentTimestampIso();
};

} // namespace core
} // namespace watchos

#endif // WATCHOS_CORE_ID_GENERATOR_HPP
