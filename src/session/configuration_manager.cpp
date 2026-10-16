#include "watchos/session/configuration_manager.hpp"
#include "watchos/process/workload_validator.hpp"
#include <iomanip>
#include <limits>
#include <sstream>

namespace watchos {
namespace session {

ConfigurationManager::ConfigurationManager(std::istream& in, std::ostream& out)
    : in_(in), out_(out) {}

int ConfigurationManager::promptInt(const std::string& promptText, int minVal, int maxVal, int defaultVal) {
    while (true) {
        out_ << promptText;
        if (defaultVal >= 0) {
            out_ << " [" << defaultVal << "]: ";
        } else {
            out_ << ": ";
        }

        std::string line;
        if (!std::getline(in_, line)) {
            return (defaultVal >= 0) ? defaultVal : minVal;
        }

        // Trim leading and trailing whitespace
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            if (defaultVal >= 0) return defaultVal;
            out_ << "[ERROR] Input cannot be blank. Please enter a valid integer.\n";
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, (last - first + 1));

        try {
            size_t idx = 0;
            int val = std::stoi(line, &idx);
            if (idx != line.length()) {
                out_ << "[ERROR] Invalid numeric input. Please enter a valid integer.\n";
                continue;
            }
            if (val < minVal || val > maxVal) {
                out_ << "[ERROR] Value must be between " << minVal << " and " << maxVal << ".\n";
                continue;
            }
            return val;
        } catch (...) {
            out_ << "[ERROR] Invalid number format. Please try again.\n";
        }
    }
}

int64_t ConfigurationManager::promptInt64(const std::string& promptText, int64_t minVal, int64_t maxVal, int64_t defaultVal) {
    while (true) {
        out_ << promptText;
        if (defaultVal >= 0) {
            out_ << " [" << defaultVal << "]: ";
        } else {
            out_ << ": ";
        }

        std::string line;
        if (!std::getline(in_, line)) {
            return (defaultVal >= 0) ? defaultVal : minVal;
        }

        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            if (defaultVal >= 0) return defaultVal;
            out_ << "[ERROR] Input cannot be blank. Please enter a valid integer.\n";
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, (last - first + 1));

        try {
            size_t idx = 0;
            long long val = std::stoll(line, &idx);
            if (idx != line.length()) {
                out_ << "[ERROR] Invalid numeric input. Please enter a valid integer.\n";
                continue;
            }
            if (val < minVal || val > maxVal) {
                out_ << "[ERROR] Value must be between " << minVal << " and " << maxVal << ".\n";
                continue;
            }
            return static_cast<int64_t>(val);
        } catch (...) {
            out_ << "[ERROR] Invalid number format. Please try again.\n";
        }
    }
}

bool ConfigurationManager::promptYesNo(const std::string& promptText, bool defaultVal) {
    while (true) {
        out_ << promptText << (defaultVal ? " (Y/n): " : " (y/N): ");
        std::string line;
        if (!std::getline(in_, line)) {
            return defaultVal;
        }
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            return defaultVal;
        }
        char c = std::tolower(line[first]);
        if (c == 'y') return true;
        if (c == 'n') return false;
        out_ << "[ERROR] Please enter 'y' for yes or 'n' for no.\n";
    }
}

std::string ConfigurationManager::promptString(const std::string& promptText, const std::string& defaultVal) {
    out_ << promptText;
    if (!defaultVal.empty()) {
        out_ << " [" << defaultVal << "]: ";
    } else {
        out_ << ": ";
    }
    std::string line;
    if (!std::getline(in_, line)) {
        return defaultVal;
    }
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return defaultVal;
    }
    size_t last = line.find_last_not_of(" \t\r\n");
    return line.substr(first, (last - first + 1));
}

bool ConfigurationManager::runInteractiveConfiguration(Session& session) {
    out_ << "\n============================================================\n";
    out_ << "           INTERACTIVE PROCESS CONFIGURATION\n";
    out_ << "============================================================\n";
    out_ << "Session ID: " << session.getSessionId() << "\n\n";

    int count = promptInt("How many processes do you want to configure?", 1, 100, 3);
    std::vector<process::Process> procs;

    for (int i = 1; i <= count; ++i) {
        auto p = promptSingleProcess(i, count, procs);
        procs.push_back(p);
    }

    session.setProcesses(procs);
    return runReviewAndEditMenu(session);
}

process::Process ConfigurationManager::promptSingleProcess(int processIndex, int totalProcesses, const std::vector<process::Process>& existing) {
    out_ << "\n--- Process " << processIndex << " of " << totalProcesses << " ---\n";

    // Auto-generate suggested PID
    int suggestedPid = 100 + processIndex;
    int pid = 0;
    while (true) {
        pid = promptInt("PID", 1, 999999, suggestedPid);
        bool duplicate = false;
        for (const auto& other : existing) {
            if (other.getPid() == pid) {
                out_ << "[ERROR] PID " << pid << " is already used by another process in this session. Enter a unique PID.\n";
                duplicate = true;
                break;
            }
        }
        if (!duplicate) break;
    }

    int64_t arrival = promptInt64("Arrival Time (>= 0)", 0, 1000000, 0);
    int64_t burst = promptInt64("CPU Burst Time (> 0)", 1, 1000000, 5);
    int priority = promptInt("Priority (validated integer, 1 = highest)", -1000, 1000, 1);

    process::Process proc(pid, arrival, burst, priority);

    // Optional subsystems
    if (promptYesNo("Configure memory requirements?", false)) {
        configureMemory(proc);
    }

    if (promptYesNo("Configure I/O requests?", false)) {
        configureIo(proc);
    }

    if (promptYesNo("Configure resource claims?", false)) {
        configureResources(proc);
    }

    if (promptYesNo("Configure disk workload?", false)) {
        configureDisk(proc);
    }

    if (promptYesNo("Configure synchronization?", false)) {
        configureSync(proc);
    }

    if (promptYesNo("Configure access permissions?", false)) {
        configureAccess(proc);
    }

    return proc;
}

void ConfigurationManager::configureMemory(process::Process& proc) {
    int64_t demand = promptInt64("  Memory demand (units > 0)", 1, 1000000, 64);
    process::MemoryConfig mem;
    mem.isConfigured = true;
    mem.demandUnits = demand;
    proc.setMemoryConfig(mem);
}

void ConfigurationManager::configureIo(process::Process& proc) {
    int numIo = promptInt("  Number of I/O requests", 1, 20, 1);
    for (int i = 1; i <= numIo; ++i) {
        out_ << "  [I/O Op " << i << " of " << numIo << "]\n";
        int64_t maxReqTime = (proc.getCpuBurst() > 1) ? (proc.getCpuBurst() - 1) : 0;
        int64_t reqTime = promptInt64("    Request time relative to CPU burst (0 <= t < " + std::to_string(proc.getCpuBurst()) + ")", 0, maxReqTime, 0);
        int64_t duration = promptInt64("    I/O duration (> 0)", 1, 1000000, 2);
        std::string device = promptString("    Device name", "DISK");
        proc.addIoRequest({reqTime, duration, device});
    }
}

void ConfigurationManager::configureResources(process::Process& proc) {
    int numRes = promptInt("  Number of resource types to claim", 1, 10, 1);
    for (int i = 1; i <= numRes; ++i) {
        out_ << "  [Resource " << i << " of " << numRes << "]\n";
        std::string rname = promptString("    Resource name (e.g., R1, MUTEX_A)", "R" + std::to_string(i));
        int maxClaim = promptInt("    Maximum claim (>= 0)", 0, 1000, 1);
        int initAlloc = promptInt("    Initial allocation (0 <= alloc <= " + std::to_string(maxClaim) + ")", 0, maxClaim, 0);
        proc.addResourceClaim({rname, maxClaim, initAlloc});
    }
}

void ConfigurationManager::configureDisk(process::Process& proc) {
    int numReq = promptInt("  Number of disk operations", 1, 20, 1);
    for (int i = 1; i <= numReq; ++i) {
        out_ << "  [Disk Op " << i << " of " << numReq << "]\n";
        int cyl = promptInt("    Target cylinder/block address (>= 0)", 0, 1000000, 50);
        std::string op = promptString("    Operation type (READ/WRITE)", "READ");
        int64_t vol = promptInt64("    Transfer volume bytes (> 0)", 1, 100000000, 4096);
        proc.addDiskRequest({cyl, op, vol});
    }
}

void ConfigurationManager::configureSync(process::Process& proc) {
    int numSync = promptInt("  Number of critical sections", 1, 10, 1);
    for (int i = 1; i <= numSync; ++i) {
        out_ << "  [Critical Section " << i << " of " << numSync << "]\n";
        std::string prim = promptString("    Primitive name (e.g. MUTEX_1)", "MUTEX_1");
        int64_t start = promptInt64("    Start time offset (>= 0)", 0, 1000000, 0);
        int64_t dur = promptInt64("    Duration in critical section (> 0)", 1, 1000000, 2);
        proc.addSyncRequirement({prim, start, dur});
    }
}

void ConfigurationManager::configureAccess(process::Process& proc) {
    std::string role = promptString("  User / Role identity", "user_1");
    std::string res = promptString("  Target resource name", "FILE_A");
    std::string op = promptString("  Requested operation (READ/WRITE/EXEC)", "READ");
    proc.setAccessConfig({true, role, res, op});
}

bool ConfigurationManager::runReviewAndEditMenu(Session& session) {
    while (true) {
        out_ << "\n============================================================\n";
        out_ << "             WORKLOAD CONFIGURATION REVIEW\n";
        out_ << "============================================================\n";
        out_ << "Session ID: " << session.getSessionId() << " | Status: " << core::sessionStatusToString(session.getStatus()) << "\n";
        out_ << "Total Processes: " << session.getProcesses().size() << "\n\n";

        // Print aligned summary table
        out_ << "+-----+---------+-----------+----------+----------+-----------------------------+\n";
        out_ << "| PID | Arrival | CPU Burst | Priority | Memory   | Subsystems Configured       |\n";
        out_ << "+-----+---------+-----------+----------+----------+-----------------------------+\n";

        if (session.getProcesses().empty()) {
            out_ << "|                    No processes configured.                               |\n";
        } else {
            for (const auto& p : session.getProcesses()) {
                std::string memStr = p.getMemoryConfig().isConfigured ? (std::to_string(p.getMemoryConfig().demandUnits) + " u") : "None";
                std::string subStr = "";
                if (!p.getIoRequests().empty()) subStr += "I/O(" + std::to_string(p.getIoRequests().size()) + ") ";
                if (!p.getResourceClaims().empty()) subStr += "Res(" + std::to_string(p.getResourceClaims().size()) + ") ";
                if (!p.getDiskRequests().empty()) subStr += "Disk(" + std::to_string(p.getDiskRequests().size()) + ") ";
                if (!p.getSyncRequirements().empty()) subStr += "Sync(" + std::to_string(p.getSyncRequirements().size()) + ") ";
                if (p.getAccessConfig().isConfigured) subStr += "Access ";
                if (subStr.empty()) subStr = "CPU-only";

                out_ << "| " << std::setw(3) << p.getPid()
                     << " | " << std::setw(7) << p.getArrivalTime()
                     << " | " << std::setw(9) << p.getCpuBurst()
                     << " | " << std::setw(8) << p.getPriority()
                     << " | " << std::setw(8) << memStr
                     << " | " << std::left << std::setw(27) << subStr << std::right << " |\n";
            }
        }
        out_ << "+-----+---------+-----------+----------+----------+-----------------------------+\n\n";

        // Check for validation issues
        auto issues = process::WorkloadValidator::validateWorkload(session.getProcesses());
        int errorCount = 0;
        for (const auto& issue : issues) {
            if (issue.severity == process::ValidationIssue::Severity::ERROR) {
                out_ << "[VALIDATION ERROR] PID " << issue.pid << " (" << issue.field << "): " << issue.message << "\n";
                errorCount++;
            }
        }
        if (errorCount > 0) {
            out_ << "\n" << errorCount << " error(s) must be resolved before finalizing.\n";
        }

        out_ << "\nWorkload Options:\n";
        out_ << "1. Edit a Process\n";
        out_ << "2. Add a New Process\n";
        out_ << "3. Remove a Process\n";
        out_ << "4. Finalize Configuration & Persist\n";
        out_ << "5. Return to Main Menu (Save Draft)\n";
        out_ << "Select an option: ";

        int choice = promptInt("", 1, 5);

        if (choice == 1) { // Edit Process
            if (session.getProcesses().empty()) {
                out_ << "[WARNING] No processes to edit.\n";
                continue;
            }
            int targetPid = promptInt("Enter PID of process to edit", 1, 999999);
            auto& procs = session.getProcesses();
            bool found = false;
            for (auto& proc : procs) {
                if (proc.getPid() == targetPid) {
                    editProcess(proc, procs);
                    found = true;
                    break;
                }
            }
            if (!found) {
                out_ << "[ERROR] Process PID " << targetPid << " not found in session.\n";
            }
        } else if (choice == 2) { // Add Process
            int newIdx = static_cast<int>(session.getProcesses().size()) + 1;
            auto p = promptSingleProcess(newIdx, newIdx, session.getProcesses());
            session.addProcess(p);
            out_ << "[OK] Process PID " << p.getPid() << " added.\n";
        } else if (choice == 3) { // Remove Process
            if (session.getProcesses().empty()) {
                out_ << "[WARNING] No processes to remove.\n";
                continue;
            }
            int targetPid = promptInt("Enter PID of process to remove", 1, 999999);
            auto& procs = session.getProcesses();
            bool removed = false;
            for (auto it = procs.begin(); it != procs.end(); ++it) {
                if (it->getPid() == targetPid) {
                    procs.erase(it);
                    removed = true;
                    out_ << "[OK] Process PID " << targetPid << " removed.\n";
                    break;
                }
            }
            if (!removed) {
                out_ << "[ERROR] Process PID " << targetPid << " not found.\n";
            }
        } else if (choice == 4) { // Finalize
            std::string err;
            if (!process::WorkloadValidator::isWorkloadValid(session.getProcesses(), err)) {
                out_ << "\n[CANNOT FINALIZE] " << err << "\n";
                out_ << "Please resolve the validation issue before finalizing.\n";
                continue;
            }
            return true; // Finalization confirmed
        } else if (choice == 5) { // Return to main menu
            return false;
        }
    }
}

void ConfigurationManager::editProcess(process::Process& proc, const std::vector<process::Process>& existing) {
    (void)existing;
    out_ << "\nEditing Process PID " << proc.getPid() << ":\n";
    int newArrival = static_cast<int>(promptInt64("New Arrival Time", 0, 1000000, proc.getArrivalTime()));
    int newBurst = static_cast<int>(promptInt64("New CPU Burst Time", 1, 1000000, proc.getCpuBurst()));
    int newPriority = promptInt("New Priority", -1000, 1000, proc.getPriority());

    proc.setArrivalTime(newArrival);
    proc.setCpuBurst(newBurst);
    proc.setPriority(newPriority);

    if (promptYesNo("Reconfigure subsystem parameters for this process?", false)) {
        if (promptYesNo("Configure memory requirements?", proc.getMemoryConfig().isConfigured)) {
            configureMemory(proc);
        } else {
            proc.setMemoryConfig(process::MemoryConfig());
        }

        if (promptYesNo("Configure I/O requests?", !proc.getIoRequests().empty())) {
            proc.clearIoRequests();
            configureIo(proc);
        } else {
            proc.clearIoRequests();
        }

        if (promptYesNo("Configure resource claims?", !proc.getResourceClaims().empty())) {
            proc.clearResourceClaims();
            configureResources(proc);
        } else {
            proc.clearResourceClaims();
        }
    }

    out_ << "[OK] Process PID " << proc.getPid() << " updated successfully.\n";
}

} // namespace session
} // namespace watchos
