#include "watchos/security/access_controller.hpp"
#include <sstream>
#include <iomanip>

namespace watchos {
namespace security {

AccessController::AccessController() {
    // Default system security policies
    addPolicy("root", "*", "*", true);
    addPolicy("user", "user_file", "READ", true);
    addPolicy("user", "user_file", "WRITE", true);
    addPolicy("user", "system_config", "READ", true);
    addPolicy("user", "system_config", "WRITE", false);
    addPolicy("guest", "user_file", "READ", true);
    addPolicy("guest", "user_file", "WRITE", false);
    addPolicy("guest", "system_config", "READ", false);
}

void AccessController::addPolicy(const std::string& roleOrUser, const std::string& resource, const std::string& operation, bool allow) {
    std::string key = roleOrUser + ":" + resource + ":" + operation;
    acl_[key] = allow;
}

void AccessController::assignRole(core::ProcessId pid, const std::string& role) {
    processRoles_[pid] = role;
}

AccessDecision AccessController::checkAccess(core::ProcessId pid, const std::string& resource, const std::string& operation, core::TimeUnits time) {
    std::string role = "user";
    auto rIt = processRoles_.find(pid);
    if (rIt != processRoles_.end()) {
        role = rIt->second;
    }

    AccessDecision dec;
    dec.actor = "PID " + std::to_string(pid) + " (" + role + ")";
    dec.resource = resource;
    dec.operation = operation;

    // Check exact rule, then wildcard rules
    std::string exactKey = role + ":" + resource + ":" + operation;
    std::string wildcardResKey = role + ":*:" + operation;
    std::string wildcardOpKey = role + ":" + resource + ":*";
    std::string superKey = role + ":*:*";

    if (acl_.find(exactKey) != acl_.end()) {
        dec.granted = acl_[exactKey];
        dec.rule = exactKey;
        dec.reason = dec.granted ? "Explicit grant rule matched" : "Explicit deny rule matched";
    } else if (acl_.find(wildcardResKey) != acl_.end()) {
        dec.granted = acl_[wildcardResKey];
        dec.rule = wildcardResKey;
        dec.reason = dec.granted ? "Wildcard resource permission granted" : "Wildcard resource denied";
    } else if (acl_.find(wildcardOpKey) != acl_.end()) {
        dec.granted = acl_[wildcardOpKey];
        dec.rule = wildcardOpKey;
        dec.reason = dec.granted ? "Wildcard operation permission granted" : "Wildcard operation denied";
    } else if (acl_.find(superKey) != acl_.end()) {
        dec.granted = acl_[superKey];
        dec.rule = superKey;
        dec.reason = dec.granted ? "Superuser wildcard permission granted" : "Superuser wildcard denied";
    } else {
        // Default deny
        dec.granted = false;
        dec.rule = "DEFAULT_DENY";
        dec.reason = "No matching access rule found. Access denied by default policy.";
    }

    auditLog_.push_back({time, pid, role, resource, operation, dec.granted, dec.reason});
    return dec;
}

bool AccessController::checkMemoryAccess(core::ProcessId pid, int64_t targetAddress, int64_t baseAddress, int64_t limit, core::TimeUnits time) {
    std::string role = processRoles_.count(pid) ? processRoles_[pid] : "user";
    bool withinBounds = (targetAddress >= baseAddress && targetAddress < (baseAddress + limit));

    AuditRecord record;
    record.timestamp = time;
    record.pid = pid;
    record.actor = role;
    record.resource = "MEM[" + std::to_string(targetAddress) + "]";
    record.operation = "MEM_ACCESS";
    record.granted = withinBounds;

    if (withinBounds) {
        record.explanation = "Address within process partition bounds [" + std::to_string(baseAddress) + ", " + std::to_string(baseAddress + limit) + ")";
    } else {
        record.explanation = "[SEGMENTATION FAULT] Out of partition bounds! Base=" + std::to_string(baseAddress) + ", Limit=" + std::to_string(limit);
    }

    auditLog_.push_back(record);
    return withinBounds;
}

std::string AccessController::formatAuditTable() const {
    std::ostringstream oss;
    oss << "Simulated Protection & Access Control Audit Log:\n";
    oss << "+------+-----+-------+----------------------+-----------+----------+--------------------------------------+\n";
    oss << "| Time | PID | Role  | Target Resource      | Operation | Decision | Reason / Details                     |\n";
    oss << "+------+-----+-------+----------------------+-----------+----------+--------------------------------------+\n";

    for (const auto& a : auditLog_) {
        oss << "| " << std::setw(4) << a.timestamp
            << " | " << std::setw(3) << a.pid
            << " | " << std::left << std::setw(5) << a.actor << std::right
            << " | " << std::left << std::setw(20) << a.resource.substr(0, 20) << std::right
            << " | " << std::left << std::setw(9) << a.operation << std::right
            << " | " << std::setw(8) << (a.granted ? "ALLOW" : "DENY")
            << " | " << std::left << std::setw(36) << a.explanation.substr(0, 36) << std::right << " |\n";
    }
    oss << "+------+-----+-------+----------------------+-----------+----------+--------------------------------------+\n";
    oss << "Disclaimer: Permissions are educational simulation rules and do not enforce host OS permissions.\n";
    return oss.str();
}

} // namespace security
} // namespace watchos
