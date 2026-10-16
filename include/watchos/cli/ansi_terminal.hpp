#ifndef WATCHOS_CLI_ANSI_TERMINAL_HPP
#define WATCHOS_CLI_ANSI_TERMINAL_HPP

#include <string>

namespace watchos {
namespace cli {

class AnsiTerminal {
public:
    static void initTerminal();
    static void setEnabled(bool enabled);
    static bool isEnabled();

    static std::string reset();
    static std::string bold();
    static std::string dim();
    static std::string cyan();
    static std::string green();
    static std::string yellow();
    static std::string red();
    static std::string blue();
    static std::string magenta();

private:
    static bool enabled_;
};

} // namespace cli
} // namespace watchos

#endif // WATCHOS_CLI_ANSI_TERMINAL_HPP
