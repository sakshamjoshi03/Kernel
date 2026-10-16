#include "watchos/cli/console_ui.hpp"
#include "watchos/cli/ansi_terminal.hpp"
#include <iostream>
#include <iomanip>

namespace watchos {
namespace cli {

void ConsoleUi::printStartupBanner() {
    std::cout << AnsiTerminal::cyan() << AnsiTerminal::bold()
              << "============================================================\n"
              << "                         WATCH OS\n"
              << "           Kernel Simulation & Analysis Engine\n"
              << "============================================================\n"
              << AnsiTerminal::reset();
}

void ConsoleUi::printBootSequence() {
    std::cout << AnsiTerminal::dim() << "[BOOT] Initializing WATCH OS...\n"
              << "[CORE] Loading simulation configuration...\n"
              << "[DB]   Connecting to session database...\n"
              << "[CORE] Validating stored sessions...\n"
              << AnsiTerminal::reset()
              << AnsiTerminal::green() << "\nSystem ready.\n" << AnsiTerminal::reset();
}

void ConsoleUi::printMainMenu() {
    std::cout << "\n"
              << AnsiTerminal::bold() << "1." << AnsiTerminal::reset() << " Create New Session\n"
              << AnsiTerminal::bold() << "2." << AnsiTerminal::reset() << " Resume Existing Session\n"
              << AnsiTerminal::bold() << "3." << AnsiTerminal::reset() << " View Session History\n"
              << AnsiTerminal::bold() << "4." << AnsiTerminal::reset() << " Compare Previous Sessions\n"
              << AnsiTerminal::bold() << "5." << AnsiTerminal::reset() << " Settings & Configuration\n"
              << AnsiTerminal::bold() << "6." << AnsiTerminal::reset() << " Exit\n\n"
              << "Select an option: ";
}

void ConsoleUi::printSessionTable(const std::vector<session::SessionMetadata>& sessions) {
    if (sessions.empty()) {
        std::cout << "\n[HISTORY] No saved sessions found in database.\n";
        return;
    }

    std::cout << "\n" << AnsiTerminal::cyan() << AnsiTerminal::bold()
              << "+-----+---------------------+-----------------------+-----------+----------+------+\n"
              << "| No  | Session ID          | Created At (UTC)      | Status    | Procs    | Runs |\n"
              << "+-----+---------------------+-----------------------+-----------+----------+------+\n"
              << AnsiTerminal::reset();

    int idx = 1;
    for (const auto& s : sessions) {
        std::string statusCol = core::sessionStatusToString(s.status);
        std::cout << "| " << std::setw(3) << idx++
                  << " | " << std::setw(19) << s.sessionId
                  << " | " << std::setw(21) << s.createdAt
                  << " | " << std::setw(9) << statusCol
                  << " | " << std::setw(8) << s.processCount
                  << " | " << std::setw(4) << s.runCount << " |\n";
    }

    std::cout << "+-----+---------------------+-----------------------+-----------+----------+------+\n";
}

void ConsoleUi::printWorkloadTable(const std::vector<process::Process>& processes) {
    std::cout << "+-----+---------+-----------+----------+----------+-----------------------------+\n";
    std::cout << "| PID | Arrival | CPU Burst | Priority | Memory   | Subsystems Configured       |\n";
    std::cout << "+-----+---------+-----------+----------+----------+-----------------------------+\n";

    if (processes.empty()) {
        std::cout << "|                    No processes configured.                               |\n";
    } else {
        for (const auto& p : processes) {
            std::string memStr = p.getMemoryConfig().isConfigured ? (std::to_string(p.getMemoryConfig().demandUnits) + " u") : "None";
            std::string subStr = "";
            if (!p.getIoRequests().empty()) subStr += "I/O(" + std::to_string(p.getIoRequests().size()) + ") ";
            if (!p.getResourceClaims().empty()) subStr += "Res(" + std::to_string(p.getResourceClaims().size()) + ") ";
            if (!p.getDiskRequests().empty()) subStr += "Disk(" + std::to_string(p.getDiskRequests().size()) + ") ";
            if (!p.getSyncRequirements().empty()) subStr += "Sync(" + std::to_string(p.getSyncRequirements().size()) + ") ";
            if (p.getAccessConfig().isConfigured) subStr += "Access ";
            if (subStr.empty()) subStr = "CPU-only";

            std::cout << "| " << std::setw(3) << p.getPid()
                      << " | " << std::setw(7) << p.getArrivalTime()
                      << " | " << std::setw(9) << p.getCpuBurst()
                      << " | " << std::setw(8) << p.getPriority()
                      << " | " << std::setw(8) << memStr
                      << " | " << std::left << std::setw(27) << subStr << std::right << " |\n";
        }
    }
    std::cout << "+-----+---------+-----------+----------+----------+-----------------------------+\n";
}

void ConsoleUi::printConfirmation(const std::string& sessionId) {
    std::cout << "\n" << AnsiTerminal::green()
              << "[SESSION] Configuration saved successfully.\n"
              << "[SESSION] Session ID: " << sessionId << "\n"
              << "[DB] All configured inputs persisted.\n\n"
              << "Configuration complete.\n"
              << "Okay, noted. You may proceed with your next query\n"
              << "or request an analysis.\n"
              << AnsiTerminal::reset();
}

void ConsoleUi::printInfo(const std::string& msg) {
    std::cout << AnsiTerminal::cyan() << "[INFO] " << msg << AnsiTerminal::reset() << "\n";
}

void ConsoleUi::printWarning(const std::string& msg) {
    std::cout << AnsiTerminal::yellow() << "[WARNING] " << msg << AnsiTerminal::reset() << "\n";
}

void ConsoleUi::printError(const std::string& msg) {
    std::cout << AnsiTerminal::red() << "[ERROR] " << msg << AnsiTerminal::reset() << "\n";
}

void ConsoleUi::printSuccess(const std::string& msg) {
    std::cout << AnsiTerminal::green() << "[SUCCESS] " << msg << AnsiTerminal::reset() << "\n";
}

} // namespace cli
} // namespace watchos
