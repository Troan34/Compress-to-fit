export module util:text_style;

import std;
import :fixed_string;

export namespace ANSI
{
    constexpr std::string_view RED = "\033[31m";
    constexpr std::string_view GREEN = "\033[32m";
    constexpr std::string_view BLUE = "\033[34m";
    constexpr std::string_view CYAN = "\033[36m";
    constexpr std::string_view MAGENTA = "\033[35m";
    constexpr std::string_view YELLOW = "\033[33m";
    constexpr std::string_view WHITE = "\033[37m";
    constexpr std::string_view RESET = "\033[0m";
    constexpr std::string_view BOLD = "\033[1m";
    constexpr std::string_view UNDERLINE = "\033[4m";
    constexpr std::string_view INVERSE = "\033[7m";
    constexpr std::string_view BOLD_RED = "\033[1;31m";
    constexpr std::string_view BOLD_GREEN = "\033[1;32m";
    constexpr std::string_view BOLD_YELLOW = "\033[1;33m";
    constexpr std::string_view BOLD_BLUE = "\033[1;34m";
    constexpr std::string_view BOLD_MAGENTA = "\033[1;35m";
    constexpr std::string_view BOLD_CYAN = "\033[1;36m";
    constexpr std::string_view BOLD_WHITE = "\033[1;37m";
}

export [[nodiscard]] constexpr auto style(std::string_view const prefix, std::string_view const string, std::string_view const suffix = ANSI::RESET)
{
    return std::string{prefix} + std::string{string} + std::string{suffix};
}

