#pragma once
#include <QObject>
#include <qqmlintegration.h>
#include <QUrl>
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
    Q_PROPERTY(CompType compressor READ compressor WRITE setCompressor)
    Q_PROPERTY(CompPreset compressorPreset READ compressorPreset WRITE setCompressorPreset)
    Q_PROPERTY(bool forceCompression READ forceCompression WRITE setForceCompression)
    Q_PROPERTY(bool deleteInput READ deleteInput WRITE setDeleteInput)
    Q_PROPERTY(ErrorType errorType READ errorType NOTIFY errorTypeChanged)

public:
    explicit CompressConfig(QObject *parent = nullptr);

    void setPathsIn(QList<QUrl> const& paths);
    void setPathOut(QUrl const& path);
    void setCompressor(CompType comp_type);
    void setCompressorPreset(CompPreset comp_preset);
    void setForceCompression(bool force_compression);
    void setDeleteInput(bool delete_input);
    void setErrorType(ErrorType error_type);

    Q_ENUM(CompType)
    Q_ENUM(CompPreset)
    Q_ENUM(ErrorType)

    [[nodiscard]] auto pathsIn() const -> QList<QUrl>;
    [[nodiscard]] auto pathOut() const -> QUrl;
    [[nodiscard]] auto compressor() const -> CompType;
    [[nodiscard]] auto compressorPreset() const -> CompPreset;
    [[nodiscard]] auto forceCompression() const -> bool;
    [[nodiscard]] auto deleteInput() const -> bool;
    [[nodiscard]] auto errorType() const -> ErrorType;

signals:
    void pathsInChanged();
    void pathOutChanged();
    /*
    void compressorChanged();
    void compressorPresetChanged();
    void forceCompressionChanged();
    void deleteInputChanged();
    */
    void errorTypeChanged();

private:
    QList<QUrl> paths_in_;
    QUrl path_out_{};
    CompType comp_type_;
    CompPreset comp_preset_;
    bool force_compression_ = false;
    bool delete_input_ = false;
    ErrorType error_type_{ErrorType::NO_ERROR};
};
