/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/

#include "fs_helper.hpp"
import std;
import util.core_utils;

auto FileSystem::isDirectory(QUrl const& path) -> bool
{
    return std::filesystem::is_directory(path.toLocalFile().toStdString());
}

auto FileSystem::isFile(QUrl const& path) -> bool
{
    return std::filesystem::is_regular_file(path.toLocalFile().toStdString());
}

auto FileSystem::extension(QUrl const& path) -> QString
{
    return QString::fromStdString(std::filesystem::path(path.toLocalFile().toStdString()).extension());
}

auto FileSystem::exists(QUrl const &path) -> bool
{
    return std::filesystem::exists(path.toLocalFile().toStdString());
}

auto FileSystem::fileName(QUrl const &path) -> QString
{
    return path.fileName();
}

auto FileSystem::fileSize(QUrl const &path) -> qsizetype
{
    return std::filesystem::file_size(path.toLocalFile().toStdString());
}

auto FileSystem::toUnit(quint64 const size) -> QString
{
    if (size < 10_KiB)//B
    {
        return QString::fromStdString(std::format("{}B", size));
    }
    if (size < 2_MiB)
    {
        return QString::fromStdString(std::format("{:g} KiB", static_cast<double>(size) / 1_KiB));
    }
    if (size < 2_GiB)
    {
        return QString::fromStdString(std::format("{:g} MiB", static_cast<double>(size) / 1_MiB));
    }
    if (size < 2_TiB)
    {
        return QString::fromStdString(std::format("{:g} GiB", static_cast<double>(size) / 1_GiB));
    }
    if (size < 2_PiB)
    {
        return QString::fromStdString(std::format("{:g} TiB", static_cast<double>(size) / 1_TiB));
    }
    if (size < 2_EiB)
    {
        return QString::fromStdString(std::format("{:g} PiB", static_cast<double>(size) / 1_PiB));
    }
}
