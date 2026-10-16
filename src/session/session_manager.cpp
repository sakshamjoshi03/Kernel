#include "watchos/session/session_manager.hpp"
#include "watchos/core/id_generator.hpp"
#include "watchos/process/workload_validator.hpp"
#include <iostream>

namespace watchos {
namespace session {

SessionManager::SessionManager(database::DbConnection& db)
    : repository_(db), activeSession_(nullptr) {}

std::shared_ptr<Session> SessionManager::createNewSession(const std::string& description) {
    // Generate unique collision-resistant ID
    std::string sessionId;
    int attempts = 0;
    do {
        sessionId = core::IdGenerator::generateSessionId();
        attempts++;
        if (attempts > 50) {
            throw std::runtime_error("Failed to generate unique Session ID after multiple attempts");
        }
    } while (repository_.sessionExists(sessionId));

    auto session = std::make_shared<Session>(sessionId, description);
    repository_.createSession(*session);
    repository_.logEvent(sessionId, "INFO", "SessionManager", "New session created: " + sessionId);

    activeSession_ = session;
    return activeSession_;
}

std::shared_ptr<Session> SessionManager::resumeSession(const std::string& sessionId) {
    auto session = std::make_shared<Session>();
    if (!repository_.loadSession(sessionId, *session)) {
        return nullptr;
    }

    activeSession_ = session;
    repository_.logEvent(sessionId, "INFO", "SessionManager", "Session resumed: " + sessionId);
    return activeSession_;
}

bool SessionManager::saveActiveSession(bool finalize, std::string* outError) {
    if (!activeSession_) {
        if (outError) *outError = "No active session loaded.";
        return false;
    }

    if (finalize) {
        std::string valErr;
        if (!process::WorkloadValidator::isWorkloadValid(activeSession_->getProcesses(), valErr)) {
            if (outError) *outError = "Validation failed: " + valErr;
            return false;
        }
        activeSession_->setStatus(core::SessionStatus::FINALIZED);
    }

    try {
        repository_.saveWorkload(*activeSession_, finalize);
        repository_.logEvent(
            activeSession_->getSessionId(),
            "INFO",
            "SessionManager",
            finalize ? "Session configuration finalized." : "Session configuration draft saved."
        );
        return true;
    } catch (const std::exception& ex) {
        if (outError) *outError = ex.what();
        return false;
    }
}

void SessionManager::closeActiveSession() {
    if (activeSession_) {
        repository_.logEvent(activeSession_->getSessionId(), "INFO", "SessionManager", "Active session closed.");
        activeSession_.reset();
    }
}

std::vector<SessionMetadata> SessionManager::listSessions() {
    return repository_.listSessions();
}

bool SessionManager::sessionExists(const std::string& sessionId) {
    return repository_.sessionExists(sessionId);
}

bool SessionManager::deleteSession(const std::string& sessionId) {
    if (activeSession_ && activeSession_->getSessionId() == sessionId) {
        closeActiveSession();
    }
    return repository_.deleteSession(sessionId);
}

} // namespace session
} // namespace watchos
