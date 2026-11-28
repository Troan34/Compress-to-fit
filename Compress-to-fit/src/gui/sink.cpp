/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Implement the GUI-side info sink
*/

#include "sink.hpp"

GuiSink::GuiSink(QObject *parent)
{}

void GuiSink::report(Status const& status) noexcept(false)
{
    if (std::holds_alternative<ErrorType>(status.message_ID_or_progress))
    {
        emit errorReceived(std::to_underlying(std::get<ErrorType>(status.message_ID_or_progress)),
        QString::fromStdString(status.failing_option.value_or("")));
    }
    else if (std::holds_alternative<WarningType>(status.message_ID_or_progress))
    {
        emit warningReceived(std::to_underlying(std::get<WarningType>(status.message_ID_or_progress)),
           QString::fromStdString(status.failing_option.value_or("")));
    }
    else if (std::holds_alternative<ProgressType>(status.message_ID_or_progress))
    {
        emit progressReceived(std::get<ProgressType>(status.message_ID_or_progress).progress,
           status.compressing.value());
    }
}

