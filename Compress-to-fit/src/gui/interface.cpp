#include "interface.hpp"
import std;
import parser;

CompressConfig::CompressConfig(QObject *parent)
    : path_out_(parser::DEFAULT_OUT_PATH.c_str())
{
}

void CompressConfig::setPathIn(QUrl const& path)
{
    path_in_ = path;
    std::println("{}", path_in_.toString().toStdString());
    emit pathInChanged();
}
void CompressConfig::setPathOut(QUrl const& path)
{
    path_out_ = path;
    emit pathOutChanged();
}
void CompressConfig::setCompressor(CompType const comp_type)
{
    comp_type_ = comp_type;
}
void CompressConfig::setCompressorPreset(CompPreset const comp_preset)
{
    comp_preset_ = comp_preset;
}
void CompressConfig::setForceCompression(bool const force_compression)
{
    force_compression_ = force_compression;
}
void CompressConfig::setDeleteInput(bool const delete_input)
{
    delete_input_ = delete_input;
}
void CompressConfig::setErrorType(ErrorType const error_type)
{
    error_type_ = error_type;
    emit errorTypeChanged();
}


auto CompressConfig::pathIn() const -> QUrl
{
    return path_in_;
}
auto CompressConfig::pathOut() const -> QUrl
{
    return path_out_;
}
auto CompressConfig::compressor() const -> CompType
{
    return comp_type_;
}
auto CompressConfig::compressorPreset() const -> CompPreset
{
    return comp_preset_;
}
auto CompressConfig::forceCompression() const -> bool
{
    return force_compression_;
}
auto CompressConfig::deleteInput() const -> bool
{
    return delete_input_;
}
auto CompressConfig::errorType() const -> ErrorType
{
    return error_type_;
}
