#pragma once
import util;
#include <QObject>
#include <QUrl>



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
class CompressOptions
{
    Q_GADGET
    Q_PROPERTY(QUrl path_in READ pathIn WRITE setPathIn)
    Q_PROPERTY(QUrl path_out READ pathOut WRITE setPathOut)
    Q_PROPERTY(Compressor comp_type READ compressor WRITE setCompressor)
    Q_PROPERTY(CompressorPreset comp_preset READ compressorPreset WRITE setCompressorPreset)
    Q_PROPERTY(bool force_compression READ forceCompression WRITE setForceCompression)
    Q_PROPERTY(bool delete_input READ deleteInput WRITE setDeleteInput)
public:
    CompressOptions();

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

private:
    QUrl path_in_{};
    QUrl path_out_{};
    Compressor comp_type_;
    CompressorPreset comp_preset_;
    bool force_compression_ = false;
    bool delete_input_ = false;
};
