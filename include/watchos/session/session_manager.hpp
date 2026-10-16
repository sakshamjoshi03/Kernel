#ifndef WATCHOS_SESSION_SESSION_MANAGER_HPP
#define WATCHOS_SESSION_SESSION_MANAGER_HPP

#include "watchos/session/session.hpp"
#include "watchos/database/session_repository.hpp"
#include "watchos/database/db_connection.hpp"
#include <memory>
#include <string>
#include <vector>

namespace watchos {
namespace session {

class SessionManager {
public:
    explicit SessionManager(database::DbConnection& db);

    // Create a new session with unique collision-resistant ID
    std::shared_ptr<Session> createNewSession(const std::string& description = "");

    // Resume existing session by ID
    std::shared_ptr<Session> resumeSession(const std::string& sessionId);

    // Get current active session
    std::shared_ptr<Session> getActiveSession() const { return activeSession_; }

    // Check if an active session is loaded
    bool hasActiveSession() const { return activeSession_ != nullptr; }

    // Save active session configuration
    bool saveActiveSession(bool finalize = false, std::string* outError = nullptr);

    // Close active session
    void closeActiveSession();

    // List all sessions in database
    std::vector<SessionMetadata> listSessions();

    // Check if a session exists
    bool sessionExists(const std::string& sessionId);

    // Delete session
    bool deleteSession(const std::string& sessionId);

    // Access underlying repository
    database::SessionRepository& getRepository() { return repository_; }

private:
    database::SessionRepository repository_;
    std::shared_ptr<Session> activeSession_;
};

} // namespace session
} // namespace watchos

#endif // WATCHOS_SESSION_SESSION_MANAGER_HPP
