/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Implement the GUI-side info sink
*/
#pragma once

#include <qqmlintegration.h>

#include "common.hpp"
#include "../common/sink.hpp"



class GuiSink : public QObject, public StatusSink
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    QML_UNCREATABLE("Provided via setExternalSingletonInstance")

public:
    explicit GuiSink(QObject *parent = nullptr);
    void report(Status const& status) noexcept(false) override;

signals:
    void errorReceived(ErrorType error, QString const& failingOption);
    void warningReceived(WarningType warning, QString const& failingOption);
    void progressReceived(float progress, bool compressing);
};