/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*   Exposes filesystem utilities to frontend
*/

#pragma once

#include <QObject>
#include <QUrl>
#include <qqmlintegration.h>

class FileSystem : public QObject
{
    Q_OBJECT

public:

    Q_INVOKABLE static auto isDirectory (QUrl const& path) -> bool;
    Q_INVOKABLE static auto isFile      (QUrl const& path) -> bool;
    Q_INVOKABLE static auto extension   (QUrl const& path) -> QString;
    Q_INVOKABLE static auto exists      (QUrl const& path) -> bool;
    Q_INVOKABLE static auto fileName    (QUrl const& path) -> QString;
    Q_INVOKABLE static auto fileSize    (QUrl const& path) -> qint64;
    Q_INVOKABLE static auto toUnit      (quint64 const size) -> QString;
};

