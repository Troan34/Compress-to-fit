/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
#pragma once
#include <QObject>
#include <qqmlintegration.h>

#include "../common/sink.hpp"
#include "src/common/comp_options.hpp"


namespace QmlEnums
{
    Q_NAMESPACE
    QML_ELEMENT

    enum class CompType : uint8_t
    {
        LZ77,
        LZ78,
        MAX,
    };
    Q_ENUM_NS(CompType)

    enum CompPreset : uint8_t
    {
        NO_COMP,
        COMP_1,
        COMP_2,
        COMP_3,
        COMP_4,
        COMP_5,
        COMP_6,
        COMP_7,
        COMP_8,
        COMP_MAX,
    };
    Q_ENUM_NS(CompPreset)

    enum class ErrorType : int
    {
        NO_ERROR,
        VALUE_ERROR,
        MISSING_ARGUMENT,
        SYNTAX_ERROR,
        OPTION_UNAVAILABLE,
        PATH_NOT_FOUND,
        PATH_NOT_ACCESSIBLE,
        PATH_INVALID,
        FILE_INVALID,
        FILE_CORRUPTED,
        DRIVE_ERROR,
        INVALID_DECOMPRESSION,
        DIR_COMPRESSION,
    };
    Q_ENUM_NS(ErrorType)

    enum class WarningType : int
    {
        RECOMPRESSION,
        PORTIONS_OUT_OF_RANGE,
        CONCAT_AMBIGUITY,
        CONCURRENCY_OUT_OF_RANGE_UPPER,
        CONCURRENCY_OUT_OF_RANGE_LOWER,
    };
    Q_ENUM_NS(WarningType)

};
