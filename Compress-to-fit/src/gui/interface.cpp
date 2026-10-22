/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
#include "interface.hpp"

#include <algorithm>
#include <utility>
import std;
import parser;
import util;

CompressConfig::CompressConfig(std::function<void(parser::Options)> compressor_callback, QObject *parent)
    : QObject(parent), path_out_(parser::DEFAULT_OUT_PATH.c_str()), compressor_callback_(std::move(compressor_callback))
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
    paths_in_ = paths;
    emit pathsInChanged();
}
void CompressConfig::setPathOut(QUrl const& path)
{
    path_out_ = path;
    emit pathOutChanged();
}
void CompressConfig::setCompressor(QmlEnums::CompType const comp_type)
{
    comp_type_ = comp_type;
    std::println("{}", static_cast<uint8_t>(comp_type));
    emit compressorChanged();
}
void CompressConfig::setCompressorPreset(QmlEnums::CompPreset const comp_preset)
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
    number_of_files_ = number_of_files;
    emit numberOfFilesChanged();
}


auto CompressConfig::pathsIn() const -> QList<QUrl>
{
    return paths_in_;
}
auto CompressConfig::pathOut() const -> QUrl
{
    return path_out_;
}
auto CompressConfig::compressor() const -> QmlEnums::CompType
{
    return comp_type_;
}
auto CompressConfig::compressorPreset() const -> QmlEnums::CompPreset
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
    return number_of_files_;
}
auto CompressConfig::isRunning() const -> bool
{
    return isRunning_;
}

auto CompressConfig::run() -> void
{
    if (isRunning_) return;
    isRunning_ = true;

    auto options = to_Options();
    std::println("{}", options);
    executor_ = std::jthread{
        [opt = std::move(options), this] mutable -> void
        {
            parser::finalize_options(opt);
            start_thread_.wait();
            compressor_callback_(opt);
        }
    };

    emit isRunningChanged();
    start_thread_.count_down();
}


auto CompressConfig::to_Options() const -> parser::Options
{
    return parser::Options{
        .filename_in = paths_in_.first().path().toStdString(),//TODO: first only because we don't support multiple paths
        .filename_out = path_out_.path().toStdString(),
        .compressor = static_cast<uint8_t>(comp_type_),
        .preset = static_cast<uint8_t>(comp_preset_),
        .n_files = number_of_files_,
        .force_compression = force_compression_,
        .delete_input = delete_input_,
    };
}

