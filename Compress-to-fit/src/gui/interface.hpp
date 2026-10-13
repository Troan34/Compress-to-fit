/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Defines the front end to compressor interface
*/
#pragma once
#include <QObject>
#include <qqmlintegration.h>
#include <QUrl>

#include "common.hpp"
#include "src/common/comp_options.hpp"
#include "src/common/error_warn_print.hpp"

/**
 * @brief parser::Options but for the frontend
 */
class CompressConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QUrl> pathsIn READ pathsIn WRITE setPathsIn NOTIFY pathsInChanged)
    Q_PROPERTY(QUrl pathOut READ pathOut WRITE setPathOut NOTIFY pathOutChanged)
    Q_PROPERTY(QmlEnums::CompType compressor READ compressor WRITE setCompressor)
    Q_PROPERTY(QmlEnums::CompPreset compressorPreset READ compressorPreset WRITE setCompressorPreset)
    Q_PROPERTY(bool forceCompression READ forceCompression WRITE setForceCompression)
    Q_PROPERTY(bool deleteInput READ deleteInput WRITE setDeleteInput)
    Q_PROPERTY(size_t numberOfFiles READ numberOfFiles WRITE setNumberOfFiles NOTIFY numberOfFilesChanged)

public:
    explicit CompressConfig(QObject *parent = nullptr);

    void setPathsIn(QList<QUrl> const& paths);
    void setPathOut(QUrl const& path);
    void setCompressor(QmlEnums::CompType comp_type);
    void setCompressorPreset(QmlEnums::CompPreset comp_preset);
    void setForceCompression(bool force_compression);
    void setDeleteInput(bool delete_input);
    void setNumberOfFiles(size_t number_of_files);


    [[nodiscard]] auto pathsIn() const -> QList<QUrl>;
    [[nodiscard]] auto pathOut() const -> QUrl;
    [[nodiscard]] auto compressor() const -> QmlEnums::CompType;
    [[nodiscard]] auto compressorPreset() const -> QmlEnums::CompPreset;
    [[nodiscard]] auto forceCompression() const -> bool;
    [[nodiscard]] auto deleteInput() const -> bool;
    [[nodiscard]] auto numberOfFiles() const -> size_t;

    Q_INVOKABLE auto run() -> void;

signals:
    void pathsInChanged();
    void pathOutChanged();
    void compressorChanged();
    void compressorPresetChanged();
    void forceCompressionChanged();
    void deleteInputChanged();
    void numberOfFilesChanged();

private:
    QList<QUrl> paths_in_;
    QUrl path_out_{};
    QmlEnums::CompType comp_type_;
    QmlEnums::CompPreset comp_preset_;
    bool force_compression_ = false;
    bool delete_input_ = false;
    size_t numberOfFiles_{};


};
