#ifndef WATCHOS_SESSION_SESSION_HPP
#define WATCHOS_SESSION_SESSION_HPP

#include "watchos/core/types.hpp"
#include "watchos/process/process.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace session {

struct SessionMetadata {
    std::string sessionId;
    std::string createdAt;
    core::SessionStatus status = core::SessionStatus::DRAFT;
    std::string description;
    int32_t activeVersion = 1;
    size_t processCount = 0;
    size_t runCount = 0;
};

class Session {
public:
    Session() = default;
    explicit Session(const std::string& sessionId, const std::string& description = "");

    const std::string& getSessionId() const { return sessionId_; }
    void setSessionId(const std::string& id) { sessionId_ = id; }

    const std::string& getCreatedAt() const { return createdAt_; }
    void setCreatedAt(const std::string& dt) { createdAt_ = dt; }

    core::SessionStatus getStatus() const { return status_; }
    void setStatus(core::SessionStatus st) { status_ = st; }

    const std::string& getDescription() const { return description_; }
    void setDescription(const std::string& desc) { description_ = desc; }

    int32_t getActiveVersion() const { return activeVersion_; }
    void setActiveVersion(int32_t ver) { activeVersion_ = ver; }

    const core::SystemSettings& getSettings() const { return settings_; }
    core::SystemSettings& getSettings() { return settings_; }
    void setSettings(const core::SystemSettings& s) { settings_ = s; }

    const std::vector<process::Process>& getProcesses() const { return processes_; }
    std::vector<process::Process>& getProcesses() { return processes_; }
    void setProcesses(const std::vector<process::Process>& procs) { processes_ = procs; }

    void addProcess(const process::Process& proc) { processes_.push_back(proc); }
    void clearProcesses() { processes_.clear(); }

private:
    std::string sessionId_;
    std::string createdAt_;
    core::SessionStatus status_ = core::SessionStatus::DRAFT;
    std::string description_;
    int32_t activeVersion_ = 1;
    core::SystemSettings settings_;
    std::vector<process::Process> processes_;
};

} // namespace session
} // namespace watchos

#endif // WATCHOS_SESSION_SESSION_HPP
