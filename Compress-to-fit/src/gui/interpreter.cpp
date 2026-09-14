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

auto interpret(std::string const& str_) -> std::expected<BackendMessage, BadMessage>
{
    auto const str = strip_ansi(str_);
    //If not an error/warning
    if (str.empty() or
        !(str.contains("Error") or str.contains("Warn")))
    {
        //if not an info on the progress
        if (!str.contains("Progress"))
        {
            return std::unexpected(BadMessage{});
        }

        auto percentage = str.substr(str.rfind(' ', str.rfind('%')), str.rfind('%') - 1);
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

    auto get_code = [str]
    {
        auto const first_bracket = str.find('[');
        auto const last_bracket = str.find(']', first_bracket);

        if (first_bracket == std::string::npos or last_bracket == std::string::npos)
        {
            return -1;
        }

        int code{};
        auto const [ptr, ec] = std::from_chars(str.data() + first_bracket + 1, str.data() + last_bracket, code);

        if (ec != std::errc())
            return -1;

        return code;
    };

    if (str.contains("Warn"))
    {

    }

    return std::unexpected(BadMessage{});
}

