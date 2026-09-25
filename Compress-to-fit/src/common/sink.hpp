/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
#pragma once
#include <variant>

#include "error_warn_print.hpp"

struct ProgressType
{
    float progress;
};

struct InputRequired{};

/**
 * @brief Package a message from the compressor
 */
struct Status
{
    std::variant<ErrorType, WarningType, ProgressType, InputRequired> message_ID_or_progress;//ID of the error/warning or amount of progress from 0 to 1
    std::optional<std::string> failing_option;//option causing error/warn, i.e. './ctf -wrongSyntax' prints -wrongSyntax <- Error[3]: the syntax...
                                              //So this contains '-wrongSyntax'

};

class StatusSink
{
public:
    virtual ~StatusSink() = default;
    virtual void report() = 0;
};


