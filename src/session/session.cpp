#include "watchos/session/session.hpp"
#include "watchos/core/id_generator.hpp"

namespace watchos {
namespace session {

Session::Session(const std::string& sessionId, const std::string& description)
    : sessionId_(sessionId),
      createdAt_(core::IdGenerator::getCurrentTimestampIso()),
      status_(core::SessionStatus::DRAFT),
      description_(description),
      activeVersion_(1) {}

} // namespace session
} // namespace watchos
