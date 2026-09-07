/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Interprets strings from the compressor.
*/
#pragma once

#include <expected>
#include <variant>
#include "src/common/error_warn_print.hpp"

struct BackendMessage
{
    std::variant<ErrorType, WarningType> message_ID;//ID of the error/warning
    std::optional<std::string> failing_option;//option causing error/warn, i.e. './ctf -wrongSyntax' prints -wrongSyntax <- Error[3]: the syntax...
                                              //does contain the preceding hyphen
    std::optional<float> progress;//from 0 to 1
};

struct BadMessage {};

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
            //parse the percentage
        }
    }
    else
    {
        //parse the error/warning
    }

}