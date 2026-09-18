/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Interprets strings from the compressor.
*/
#pragma once

#include <expected>
#include <QObject>
#include <regex>
#include <variant>
#include "src/common/error_warn_print.hpp"


/**
 * @brief Remove ANSI \x1b sequences (color, underline...)
 * @param str Input string
 * @return The string without ANSI sequences
 */
auto strip_ansi(std::string const& str) -> std::string;

/**
 * @brief Remove leading and trailing whitespace
 * @param str Input string
 * @return The trimmed string
 */
auto trim_whitespace(std::string const& str) -> std::string;

struct ProgressType
{
    float progress;
};

/**
 * @brief Package a message from the compressor
 */
struct BackendMessage
{
    std::variant<ErrorType, WarningType, ProgressType> message_ID_or_progress;//ID of the error/warning or amount of progress from 0 to 1
    std::optional<std::string> failing_option;//option causing error/warn, i.e. './ctf -wrongSyntax' prints -wrongSyntax <- Error[3]: the syntax...
                                              //So this contains '-wrongSyntax'
};

//A dummy type to check against for a bad backend message
struct BadMessage {};

/**
 * @brief Receive and interpret the compressor's output
 * @param str To be interpreted
 * @return The BackendMessage, or a BadMessage if the parsing failed
 */
auto interpret(std::string const& str) -> std::expected<BackendMessage, BadMessage>;

class QmlBackendMessage : public QObject
{
    Q_GADGET
    Q_PROPERTY(Severity severity READ severity CONSTANT)
    Q_PROPERTY(QString text READ text CONSTANT)

public:
    enum class Severity { Info, Warning, Error };
    Q_ENUM(Severity)

    explicit QmlBackendMessage(QObject *parent = nullptr);


};

Q_DECLARE_METATYPE(QmlBackendMessage)

class StatusBridge : public QObject
{
    Q_OBJECT
public:
    explicit StatusBridge(QObject* parent = nullptr) : QObject(parent) {}



signals:
    void messageReceived(BackendMessage const& message);

};



