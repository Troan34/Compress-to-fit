/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/

#pragma once
#include "../common/sink.hpp"

class StdoutSink : public StatusSink
{
public:
    void report(Status const &status) noexcept(false) override;
};