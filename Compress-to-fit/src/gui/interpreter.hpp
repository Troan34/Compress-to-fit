/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Interprets strings from the compressor.
*/
#pragma once

#include <expected>
#include <regex>
#include <variant>
#include "src/common/error_warn_print.hpp"



auto strip_ansi(const std::string& str) -> std::string;


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
 * @param str_ To be interpreted
 * @return The BackendMessage, or a BadMessage if the parsing failed
 */
auto interpret(std::string const& str_) -> std::expected<BackendMessage, BadMessage>;