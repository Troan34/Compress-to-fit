#pragma once

#include <QObject>
#include <QUrl>
#include <qqmlintegration.h>

class FileSystem : public QObject
{
    Q_OBJECT

public:

    Q_INVOKABLE auto isDirectory(QUrl const& path) -> bool;
    Q_INVOKABLE auto isFile(QUrl const& path) -> bool;
    Q_INVOKABLE auto extension(QUrl const& path) -> QString;
    Q_INVOKABLE auto exists(QUrl const& path) -> bool;
    Q_INVOKABLE auto fileName(QUrl const& path) -> QString;
};

