/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Interprets strings from the compressor.
*/

#include "interpreter.hpp"

auto strip_ansi(const std::string& str) -> std::string
{
    //This is LLM-made because I'm NOT writing a regex
    static const std::regex ansi(
        R"(\x1b\[[0-9;]*[A-Za-z])");   // CSI sequences
    return std::regex_replace(str, ansi, "");
}

auto interpret(std::string const& str) -> std::expected<BackendMessage, BadMessage>
{
    auto const stripped_str = strip_ansi(str);
    //If not an error/warning
    if (stripped_str.empty() or
        !(stripped_str.contains("Error") or stripped_str.contains("Warn")))
    {
        //if not an info on the progress
        if (!stripped_str.contains("Progress"))
        {
            return std::unexpected(BadMessage{});
        }

        auto percentage = stripped_str.substr(stripped_str.rfind(' ', stripped_str.rfind('%')), stripped_str.rfind('%') - 1);
        //get the percentage
        auto progress{0.F};
        auto const [_, ec] = std::from_chars(percentage.data(), percentage.data() + percentage.size(), progress);
        if (ec != std::errc())
        {
            return std::unexpected(BadMessage{});
        }

        //initialize the variant
        std::variant<ErrorType, WarningType, ProgressType> const message_ID_or_progress_{ProgressType{ progress }};
        //init the backend_message without the optional
        BackendMessage backend_message{.message_ID_or_progress = message_ID_or_progress_};
        return backend_message;
    }

    auto get_code = [stripped_str]
    {
        auto const first_bracket = stripped_str.find('[');
        auto const last_bracket = stripped_str.find(']', first_bracket);

        if (first_bracket == std::string::npos or last_bracket == std::string::npos)
        {
            return -1;
        }

        int code{};
        auto const [ptr, ec] = std::from_chars(stripped_str.data() + first_bracket + 1, stripped_str.data() + last_bracket, code);

        if (ec != std::errc())
            return -1;

        return code;
    };

    if (stripped_str.contains("Warn"))
    {
        auto const code = get_code();
        if (code == -1)
            return std::unexpected(BadMessage{});

        auto const arrow_index = stripped_str.rfind("<-");
        if (arrow_index == std::string::npos)
            return std::unexpected(BadMessage{});

        auto const warn_opt = stripped_str.substr(0, arrow_index);
        //TODO:remove trailing whitespace

        return BackendMessage{.message_ID_or_progress = WarningType{code}, };
    }

    return std::unexpected(BadMessage{});
}
