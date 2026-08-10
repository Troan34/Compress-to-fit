#include "fs_helper.hpp"
#include <filesystem>

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

