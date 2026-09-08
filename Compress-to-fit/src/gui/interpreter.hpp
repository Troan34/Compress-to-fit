/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Interprets strings from the compressor.
*/
#pragma once

#include <expected>
#include <variant>
#include "src/common/error_warn_print.hpp"

struct ProgressType
{
    float progress;
};

/**
 * @brief Package a message from the compressor
 */
struct BackendMessage
{
    std::variant<ErrorType, WarningType, ProgressType> message_ID_or_progress;//ID of the error/warning or amount of progress from 0 to 1
    std::optional<std::string> failing_option;//option causing error/warn, i.e. './ctf -wrongSyntax' prints -wrongSyntax <- Error[3]: the syntax...
                                              //So this contains '-wrongSyntax'
};

struct BadMessage {};

/**
 * @brief Receive and interpret the compressor's output
 * @param str To be interpreted
 * @return The BackendMessage, or a BadMessage if the parsing failed
 */
auto interpret(std::string const& str) -> std::expected<BackendMessage, BadMessage>
{
    //If not an error/warning
    if (str.empty() or
        !(str.contains("Error") or str.contains("Warn")))
    {
        //if not an info on the progress
        if (!str.contains("Progress"))
        {
            return std::unexpected(BadMessage{});
        }
        else
        {
            //get the percentage
            auto const progress = std::stof(str.substr(str.rfind(' ', str.rfind('%')), str.rfind('%') - 1));
            //initialize the variant
            std::variant<ErrorType, WarningType, ProgressType> const message_ID_or_progress_{ProgressType{ progress }};
            //init the backend_message without the optional
            BackendMessage backend_message{.message_ID_or_progress = message_ID_or_progress_};
            return backend_message;
        }
    }
    else if (str.contains("Warn"))
    {
        
    }

}