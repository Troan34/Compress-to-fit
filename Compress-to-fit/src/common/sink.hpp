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
    operator float() const { return progress; }
};


/**
 * @brief Package a message from the core
 */
struct Status
{
    std::variant<ErrorType, WarningType, ProgressType> message_ID_or_progress;//ID of the error/warning or amount of progress from 0 to 1
    std::optional<std::string> failing_option;//option causing error/warn, i.e. './ctf -wrongSyntax' prints -wrongSyntax <- Error[3]: the syntax...
                                              //So this contains '-wrongSyntax'
    std::optional<bool> compressing;
};

/**
 * @brief Sink for every type of message, customize the behavior by implementing `report`
 */
class StatusSink
{
public:
    virtual ~StatusSink() = default;

    /**
     * @brief sink the @p status
     * @param status Your status/message
     */
    virtual void report(Status const& status) = 0;
};

/**
 * @brief Singleton-like instance of StatusSink, but each thread has its own
 */
class SinkInstancer
{
public:
    explicit SinkInstancer(StatusSink& sink) : prev_(current()) { current() = &sink; }
    ~SinkInstancer() { current() = prev_; }

    SinkInstancer(const SinkInstancer&) = delete;
    SinkInstancer(SinkInstancer&&) = delete;
    auto operator=(const SinkInstancer&) -> SinkInstancer& = delete;
    auto operator=(SinkInstancer&&) -> SinkInstancer& = delete;

    static auto current() -> StatusSink*&
    {
        thread_local StatusSink* sink = nullptr;
        return sink;
    }

private:
    StatusSink* prev_;
};

/**
 * @brief Generically report @p status into the thread_local sink
 */
inline auto report(Status const& status) -> void
{
    if (auto* sink = SinkInstancer::current()) sink->report(status);
}
