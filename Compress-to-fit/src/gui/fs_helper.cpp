#include "fs_helper.hpp"
#include <filesystem>
#include <string>
#include <algorithm>
#include <cmath>

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
    if (size < 10ULL * 1024)//B
    {
        return QString::fromStdString(std::to_string(size) + "B");
    }
    else if (size < 2ULL * 1024 * 1024)//KiB when less than 2MiB
    {
        return QString::fromStdString(std::to_string(size / 1024ULL) + "KiB");
    }
    else if (size < 2ULL * 1024 * 1024 * 1024)//MiB when less than 2GiB
    {
        return QString::fromStdString(std::to_string(size / (1024ULL * 1024)) + "MiB");
    }
    else if (size < 2ULL * 1024 * 1024 * 1024 * 1024)//GiB when less than 2TiB
    {
        return QString::fromStdString(std::to_string(size / (1024ULL * 1024 * 1024)) + "GiB");
    }
    else if (size < 2ULL * 1024 * 1024 * 1024 * 1024 * 1024)//TiB when less than 2PiB
    {
        return QString::fromStdString(std::to_string(size / (1024ULL * 1024 * 1024 * 1024)) + "TiB");
    }
}
