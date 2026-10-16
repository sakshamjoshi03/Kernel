#include "watchos/cli/cli_app.hpp"
#include "watchos/cli/console_ui.hpp"
#include "watchos/cli/ansi_terminal.hpp"
#include "watchos/database/schema.hpp"
#include "watchos/session/configuration_manager.hpp"
#include "watchos/process/workload_validator.hpp"
#include "watchos/cli/analysis_menu.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace watchos {
namespace cli {

CliApp::CliApp(const std::string& dbPath) : dbPath_(dbPath) {}

int CliApp::run() {
    AnsiTerminal::initTerminal();
    ConsoleUi::printStartupBanner();
    ConsoleUi::printBootSequence();

    try {
        db_ = std::make_unique<database::DbConnection>(dbPath_);
        db_->open();
        database::Schema::initializeSchema(*db_);
        sessionManager_ = std::make_unique<session::SessionManager>(*db_);
    } catch (const std::exception& ex) {
        ConsoleUi::printError("Failed to initialize database: " + std::string(ex.what()));
        return 1;
    }

    bool running = true;
    while (running) {
        ConsoleUi::printMainMenu();

        std::string inputLine;
        if (!std::getline(std::cin, inputLine)) {
            break; // EOF or stream error
        }

        size_t first = inputLine.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            continue;
        }

        char choice = inputLine[first];
        switch (choice) {
            case '1':
                handleCreateSession();
                break;
            case '2':
                handleResumeSession();
                break;
            case '3':
                handleViewHistory();
                break;
            case '4':
                handleCompareSessions();
                break;
            case '5':
                handleSettings();
                break;
            case '6':
                ConsoleUi::printInfo("Exiting WATCH OS. All committed database records are preserved.");
                running = false;
                break;
            default:
                ConsoleUi::printWarning("Invalid menu selection '" + inputLine + "'. Please select an option between 1 and 6.");
                break;
        }
    }

    if (sessionManager_) {
        sessionManager_->closeActiveSession();
    }
    if (db_) {
        db_->close();
    }
    return 0;
}

void CliApp::handleCreateSession() {
    std::cout << "\nEnter session description (optional): ";
    std::string desc;
    std::getline(std::cin, desc);

    std::shared_ptr<session::Session> session;
    try {
        session = sessionManager_->createNewSession(desc);
    } catch (const std::exception& ex) {
        ConsoleUi::printError("Failed to create session: " + std::string(ex.what()));
        return;
    }

    ConsoleUi::printSuccess("Created new Session ID: " + session->getSessionId());

    session::ConfigurationManager configMgr(std::cin, std::cout);
    bool finalized = configMgr.runInteractiveConfiguration(*session);
    std::string saveErr;
    if (sessionManager_->saveActiveSession(finalized, &saveErr)) {
        if (finalized) {
            ConsoleUi::printConfirmation(session->getSessionId());
            AnalysisMenu analysisMenu(*session, *db_);
            analysisMenu.runMenuLoop();
        } else {
            ConsoleUi::printInfo("Workload saved as DRAFT under Session ID: " + session->getSessionId());
        }
    } else {
        ConsoleUi::printError("Could not save session: " + saveErr);
    }
}

void CliApp::handleResumeSession() {
    auto sessions = sessionManager_->listSessions();
    if (sessions.empty()) {
        ConsoleUi::printWarning("No saved sessions available to resume.");
        return;
    }

    ConsoleUi::printSessionTable(sessions);
    std::cout << "\nEnter Session ID or row number (1-" << sessions.size() << ") to resume: ";
    std::string input;
    if (!std::getline(std::cin, input) || input.empty()) {
        return;
    }

    std::string targetId;
    try {
        size_t idx = std::stoul(input);
        if (idx >= 1 && idx <= sessions.size()) {
            targetId = sessions[idx - 1].sessionId;
        }
    } catch (...) {
        targetId = input;
    }

    auto session = sessionManager_->resumeSession(targetId);
    if (!session) {
        ConsoleUi::printError("Session ID '" + targetId + "' not found.");
        return;
    }

    ConsoleUi::printSuccess("Successfully resumed Session: " + session->getSessionId());
    std::cout << "Description: " << (session->getDescription().empty() ? "(none)" : session->getDescription()) << "\n";
    std::cout << "Status: " << core::sessionStatusToString(session->getStatus()) << "\n";
    std::cout << "Configured Processes: " << session->getProcesses().size() << "\n\n";

    ConsoleUi::printWorkloadTable(session->getProcesses());

    std::cout << "\nOptions for resumed session:\n";
    std::cout << "1. Enter Analysis Console (Run Simulations)\n";
    std::cout << "2. Review and Edit Workload\n";
    std::cout << "3. Return to Main Menu\n";
    std::cout << "Select option: ";

    std::string subChoice;
    std::getline(std::cin, subChoice);
    if (!subChoice.empty() && subChoice[0] == '1') {
        AnalysisMenu analysisMenu(*session, *db_);
        analysisMenu.runMenuLoop();
    } else if (!subChoice.empty() && subChoice[0] == '2') {
        session::ConfigurationManager configMgr(std::cin, std::cout);
        bool finalized = configMgr.runReviewAndEditMenu(*session);
        std::string err;
        if (sessionManager_->saveActiveSession(finalized, &err)) {
            ConsoleUi::printSuccess("Session configuration updated successfully.");
            if (finalized) {
                AnalysisMenu analysisMenu(*session, *db_);
                analysisMenu.runMenuLoop();
            }
        } else {
            ConsoleUi::printError("Failed to update session: " + err);
        }
    }
}

void CliApp::handleViewHistory() {
    auto sessions = sessionManager_->listSessions();
    ConsoleUi::printSessionTable(sessions);
}

void CliApp::handleCompareSessions() {
    auto sessions = sessionManager_->listSessions();
    if (sessions.size() < 2) {
        ConsoleUi::printWarning("At least 2 sessions are required for comparison. Currently available: " + std::to_string(sessions.size()));
        return;
    }

    ConsoleUi::printSessionTable(sessions);
    std::cout << "\nEnter first Session row number or ID: ";
    std::string id1Str;
    std::getline(std::cin, id1Str);

    std::cout << "Enter second Session row number or ID: ";
    std::string id2Str;
    std::getline(std::cin, id2Str);

    std::string id1 = id1Str, id2 = id2Str;
    try {
        size_t idx1 = std::stoul(id1Str);
        if (idx1 >= 1 && idx1 <= sessions.size()) id1 = sessions[idx1 - 1].sessionId;
    } catch (...) {}

    try {
        size_t idx2 = std::stoul(id2Str);
        if (idx2 >= 1 && idx2 <= sessions.size()) id2 = sessions[idx2 - 1].sessionId;
    } catch (...) {}

    session::Session s1, s2;
    if (!sessionManager_->getRepository().loadSession(id1, s1) || !sessionManager_->getRepository().loadSession(id2, s2)) {
        ConsoleUi::printError("Could not load one or both sessions for comparison.");
        return;
    }

    int64_t totalBurst1 = 0, totalBurst2 = 0;
    for (const auto& p : s1.getProcesses()) totalBurst1 += p.getCpuBurst();
    for (const auto& p : s2.getProcesses()) totalBurst2 += p.getCpuBurst();

    std::cout << "\n============================================================\n";
    std::cout << "                  SESSION WORKLOAD COMPARISON\n";
    std::cout << "============================================================\n";
    std::cout << "+------------------------------+--------------------+--------------------+\n";
    std::cout << "| Metric / Parameter           | Session A          | Session B          |\n";
    std::cout << "+------------------------------+--------------------+--------------------+\n";
    std::cout << "| Session ID                   | " << std::setw(18) << s1.getSessionId() << " | " << std::setw(18) << s2.getSessionId() << " |\n";
    std::cout << "| Status                       | " << std::setw(18) << core::sessionStatusToString(s1.getStatus()) << " | " << std::setw(18) << core::sessionStatusToString(s2.getStatus()) << " |\n";
    std::cout << "| Process Count                | " << std::setw(18) << s1.getProcesses().size() << " | " << std::setw(18) << s2.getProcesses().size() << " |\n";
    std::cout << "| Total CPU Workload (units)   | " << std::setw(18) << totalBurst1 << " | " << std::setw(18) << totalBurst2 << " |\n";
    std::cout << "+------------------------------+--------------------+--------------------+\n\n";
}

void CliApp::handleSettings() {
    std::cout << "\n============================================================\n";
    std::cout << "                 SETTINGS & CONFIGURATION\n";
    std::cout << "============================================================\n";
    std::cout << "1. Default Round Robin Quantum: " << settings_.defaultTimeQuantum << "\n";
    std::cout << "2. Default Memory Frames: " << settings_.defaultMemoryFrames << "\n";
    std::cout << "3. Aging Interval: " << settings_.agingInterval << "\n";
    std::cout << "4. Aging Increment: " << settings_.agingIncrement << "\n";
    std::cout << "5. Context Switch Overhead: " << settings_.contextSwitchOverhead << "\n";
    std::cout << "6. Terminal ANSI Color: " << (settings_.enableAnsiColor ? "ENABLED" : "DISABLED") << "\n";
    std::cout << "7. Toggle ANSI Color\n";
    std::cout << "8. Return to Main Menu\n";
    std::cout << "Select option: ";

    std::string opt;
    std::getline(std::cin, opt);
    if (!opt.empty() && opt[0] == '7') {
        settings_.enableAnsiColor = !settings_.enableAnsiColor;
        AnsiTerminal::setEnabled(settings_.enableAnsiColor);
        ConsoleUi::printSuccess(std::string("ANSI Color is now ") + (settings_.enableAnsiColor ? "ENABLED" : "DISABLED"));
    }
}

} // namespace cli
} // namespace watchos
