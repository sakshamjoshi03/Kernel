#ifndef WATCHOS_SECURITY_ACCESS_CONTROLLER_HPP
#define WATCHOS_SECURITY_ACCESS_CONTROLLER_HPP

#include "watchos/core/types.hpp"
#include <string>
#include <vector>
#include <map>

namespace watchos {
namespace security {

struct AccessDecision {
    bool granted = false;
    std::string actor;
    std::string resource;
    std::string operation;
    std::string rule;
    std::string reason;
};

struct AuditRecord {
    core::TimeUnits timestamp = 0;
    core::ProcessId pid = 0;
    std::string actor;
    std::string resource;
    std::string operation;
    bool granted = false;
    std::string explanation;
};

class AccessController {
public:
    AccessController();

    void addPolicy(const std::string& roleOrUser, const std::string& resource, const std::string& operation, bool allow);
    void assignRole(core::ProcessId pid, const std::string& role);

    AccessDecision checkAccess(core::ProcessId pid, const std::string& resource, const std::string& operation, core::TimeUnits time);

    // Memory protection boundary check
    bool checkMemoryAccess(core::ProcessId pid, int64_t targetAddress, int64_t baseAddress, int64_t limit, core::TimeUnits time);

    const std::vector<AuditRecord>& getAuditLog() const { return auditLog_; }
    std::string formatAuditTable() const;

private:
    std::map<core::ProcessId, std::string> processRoles_;
    // key: role + ":" + resource + ":" + operation -> allow/deny
    std::map<std::string, bool> acl_;
    std::vector<AuditRecord> auditLog_;
};

} // namespace security
} // namespace watchos

#endif // WATCHOS_SECURITY_ACCESS_CONTROLLER_HPP
