#ifndef WATCHOS_DATABASE_SESSION_REPOSITORY_HPP
#define WATCHOS_DATABASE_SESSION_REPOSITORY_HPP

#include "watchos/database/db_connection.hpp"
#include "watchos/session/session.hpp"
#include <vector>
#include <string>

namespace watchos {
namespace database {

class SessionRepository {
public:
    explicit SessionRepository(DbConnection& db);

    // Create session record
    void createSession(const session::Session& session);

    // Save or update workload configuration under active version in a transaction
    void saveWorkload(const session::Session& session, bool finalize);

    // Load full session with processes and subsystem configs
    bool loadSession(const std::string& sessionId, session::Session& outSession);

    // List all sessions with summary metadata
    std::vector<session::SessionMetadata> listSessions();

    // Check if session exists
    bool sessionExists(const std::string& sessionId);

    // Update status
    void updateSessionStatus(const std::string& sessionId, core::SessionStatus status);

    // Delete session (cascade deletes configs, processes, runs)
    bool deleteSession(const std::string& sessionId);

    // Log operational message
    void logEvent(const std::string& sessionId, const std::string& level, const std::string& source, const std::string& message);

private:
    DbConnection& db_;

    void saveProcesses(const std::string& sessionId, int32_t version, const std::vector<process::Process>& processes);
    void loadProcesses(const std::string& sessionId, int32_t version, std::vector<process::Process>& outProcesses);
};

} // namespace database
} // namespace watchos

#endif // WATCHOS_DATABASE_SESSION_REPOSITORY_HPP
