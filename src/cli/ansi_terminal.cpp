#include "watchos/cli/ansi_terminal.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

namespace watchos {
namespace cli {

bool AnsiTerminal::enabled_ = true;

void AnsiTerminal::initTerminal() {
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

void AnsiTerminal::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool AnsiTerminal::isEnabled() {
    return enabled_;
}

std::string AnsiTerminal::reset() {
    return enabled_ ? "\033[0m" : "";
}

std::string AnsiTerminal::bold() {
    return enabled_ ? "\033[1m" : "";
}

std::string AnsiTerminal::dim() {
    return enabled_ ? "\033[2m" : "";
}

std::string AnsiTerminal::cyan() {
    return enabled_ ? "\033[36m" : "";
}

std::string AnsiTerminal::green() {
    return enabled_ ? "\033[32m" : "";
}

std::string AnsiTerminal::yellow() {
    return enabled_ ? "\033[33m" : "";
}

std::string AnsiTerminal::red() {
    return enabled_ ? "\033[31m" : "";
}

std::string AnsiTerminal::blue() {
    return enabled_ ? "\033[34m" : "";
}

std::string AnsiTerminal::magenta() {
    return enabled_ ? "\033[35m" : "";
}

} // namespace cli
} // namespace watchos
