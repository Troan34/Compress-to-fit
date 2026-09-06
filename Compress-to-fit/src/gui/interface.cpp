/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
#include "interface.hpp"

#include <algorithm>
import std;
import parser;
import util;

CompressConfig::CompressConfig(QObject *parent)
    : path_out_(parser::DEFAULT_OUT_PATH.c_str())
{
}

void CompressConfig::setPathsIn(QList<QUrl> const& paths)
{
    std::ranges::for_each(
        paths,
        [](QUrl const& path)
    {
        std::println("{}", path.toString().toStdString());
    });

    emit pathsInChanged();
}
void CompressConfig::setPathOut(QUrl const& path)
{
    path_out_ = path;
    emit pathOutChanged();
}
void CompressConfig::setCompressor(CompType const comp_type)
{
    comp_type_ = comp_type;
    emit compressorChanged();
}
void CompressConfig::setCompressorPreset(CompPreset const comp_preset)
{
    comp_preset_ = comp_preset;
    emit compressorPresetChanged();
}
void CompressConfig::setForceCompression(bool const force_compression)
{
    force_compression_ = force_compression;
    emit forceCompressionChanged();
}
void CompressConfig::setDeleteInput(bool const delete_input)
{
    delete_input_ = delete_input;
    emit deleteInputChanged();
}
void CompressConfig::setNumberOfFiles(size_t const number_of_files)
{
    numberOfFiles_ = number_of_files;
    emit numberOfFilesChanged();
}
void CompressConfig::setErrorType(ErrorType const error_type)
{
    error_type_ = error_type;
    emit errorTypeChanged();
}


auto CompressConfig::pathsIn() const -> QList<QUrl>
{
    return paths_in_;
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
auto CompressConfig::numberOfFiles() const -> size_t
{
    return numberOfFiles_;
}
auto CompressConfig::errorType() const -> ErrorType
{
    return error_type_;
}
