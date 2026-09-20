/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Interprets strings from the compressor.
*/

#include "interpreter.hpp"
#include <utility>

auto strip_ansi(std::string const& str) -> std::string
{
    //This is LLM-made because I'm NOT writing a regex
    //I learnt basic regex just to explain the following:
    //\x1b is a literal, \[ is a literal, [0-9;]* is basically <anynum or ;>repeated, [A-Za-z] means the whole alphabet
    static const std::regex ansi(
        R"(\x1b\[[0-9;]*[A-Za-z])");   // CSI sequences
    return std::regex_replace(str, ansi, "");
}

auto trim_whitespace(std::string const& str) -> std::string
{
    constexpr auto WHITESPACE{" \n\r\t\f\v"};
    auto const start = str.find_first_not_of(WHITESPACE);
    if (start == std::string::npos)
        return "";

    auto const end = str.find_last_not_of(WHITESPACE);
    if (end == std::string::npos)
        std::unreachable();

    return str.substr(start, end + 1);
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

    //Get the code between the [<code>]
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

    //Get the failing option that is behind the '<-'
    auto get_failing_opt = [stripped_str] -> std::expected<std::string, BadMessage>
    {
        auto const arrow_index = stripped_str.rfind("<-");//The user might insert (GUI wise) something like "-i <-" (I have no idea if that is possible)
                                                                   //It's important that we don't have a warn/err string with <-
        if (arrow_index == std::string::npos)
            return std::unexpected(BadMessage{});

        return trim_whitespace(stripped_str.substr(0, arrow_index));
    };

    auto const code = get_code();
    if (code == -1)
        return std::unexpected(BadMessage{});

    auto const result = get_failing_opt();
    if (!result.has_value())
        return std::unexpected(result.error());

    if (stripped_str.contains("Warn"))
    {
        return BackendMessage{ .message_ID_or_progress = WarningType{code},
                                  .failing_option = result.value(), };
    }

    if (stripped_str.contains("Error"))
    {
        return BackendMessage{ .message_ID_or_progress = ErrorType{code},
                                  .failing_option = result.value(), };
    }

    return std::unexpected(BadMessage{});
}


auto QmlBackendMessage::severity() const -> Severity
{
    return severity_;
}

auto QmlBackendMessage::text() const -> QString
{
    return text_;
}

auto QmlBackendMessage::ID() const -> qint32
{
    return ID_;
}