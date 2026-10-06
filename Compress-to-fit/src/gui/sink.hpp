/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Implement the GUI-side info sink
*/
#pragma once

#include "common.hpp"
#include "../common/sink.hpp"

class GuiStatus
{
    Q_GADGET
public:


signals:
    void errorReceived(ErrorType error, QString failingOption);
    void warningReceived(WarningType warning, QString failingOption);
    void progressReceived(ProgressType progress, QString failingOption);
};

class GuiSink : public QObject, public StatusSink
{
    Q_OBJECT

public:


};