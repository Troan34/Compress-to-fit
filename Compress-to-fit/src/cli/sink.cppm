/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/

export module sink;

#include "../common/sink.hpp"
import std;
import parser;

export class StdoutSink : public StatusSink
{
public:
    void report(Status const &status) noexcept(false) override;
};


void StdoutSink::report(Status const& status) noexcept(false) override
{
    if (std::holds_alternative<ErrorType>(status.message_ID_or_progress))
        throw_error(std::visit<ErrorType>(status.message_ID_or_progress), status.failing_option.value_or(""));
    else if (std::holds_alternative<WarningType>(status.message_ID_or_progress))
        print_warn(std::visit<WarningType>(status.message_ID_or_progress), status.failing_option.value_or(""));
    else if (std::holds_alternative<ProgressType>(status.message_ID_or_progress))
        show_progress({}, std::visit<ProgressType>(status.message_ID_or_progress), status.compressing.value());
}
