/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
#pragma once
#include <QObject>

#include "../common/sink.hpp"


namespace common
{
    using ::ErrorType;
    using ::WarningType;
    Q_NAMESPACE
    Q_ENUM_NS(ErrorType)
    Q_ENUM_NS(WarningType)
}
