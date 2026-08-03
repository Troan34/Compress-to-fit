#pragma once
#include <QObject>
#include <qqmlintegration.h>
#include <QUrl>
#include "src/common/comp_options.hpp"

class Compressor
{
    Q_GADGET

public:
    Compressor() = default;

    Q_ENUM(CompType)
};

class CompressorPreset
{
    Q_GADGET

public:
    CompressorPreset() = default;

    Q_ENUM(CompPreset)
};


/**
 * @brief parser::Options but for the frontend
 */
class CompressConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl pathIn READ pathIn WRITE setPathIn NOTIFY pathInChanged)
    Q_PROPERTY(QUrl pathOut READ pathOut WRITE setPathOut NOTIFY pathOutChanged)
    Q_PROPERTY(Compressor compType READ compressor WRITE setCompressor)
    Q_PROPERTY(CompressorPreset compType READ compressorPreset WRITE setCompressorPreset)
    Q_PROPERTY(bool forceCompression READ forceCompression WRITE setForceCompression)
    Q_PROPERTY(bool deleteInput READ deleteInput WRITE setDeleteInput)

public:
    explicit CompressConfig(QObject *parent = nullptr);

    void setPathIn(QUrl const& path);
    void setPathOut(QUrl const& path);
    void setCompressor(Compressor comp_type);
    void setCompressorPreset(CompressorPreset comp_preset);
    void setForceCompression(bool force_compression);
    void setDeleteInput(bool delete_input);


    [[nodiscard]] auto pathIn() const -> QUrl;
    [[nodiscard]] auto pathOut() const -> QUrl;
    [[nodiscard]] auto compressor() const -> Compressor;
    [[nodiscard]] auto compressorPreset() const -> CompressorPreset;
    [[nodiscard]] auto forceCompression() const -> bool;
    [[nodiscard]] auto deleteInput() const -> bool;

signals:
    void pathInChanged();
    void pathOutChanged();

private:
    QUrl path_in_{};
    QUrl path_out_{};
    Compressor comp_type_;
    CompressorPreset comp_preset_;
    bool force_compression_ = false;
    bool delete_input_ = false;
};
