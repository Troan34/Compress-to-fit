/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
import std;

#include "sink.hpp"

void StdoutSink::report(Status const &status) noexcept(false) override
{
    if (std::holds_alternative<ErrorType>(status.message_ID_or_progress))
    {
        throw_error()
    }
}
