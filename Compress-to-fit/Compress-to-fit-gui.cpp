#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QQmlContext>
#include <QDebug>
#include "src/gui/interface.hpp"
#include "src/gui/fs_helper.hpp"

import util;

import parser;
import models;
import util;
import std.compat;

namespace fs = std::filesystem;


int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);


    QQmlApplicationEngine engine;

    FileSystem file_system;
    CompressConfig compressor_conf;
    engine.rootContext()->setContextProperty("compressor_conf", &compressor_conf);
    engine.rootContext()->setContextProperty("fileSystem", &file_system);


    qDebug() << "Qt version:" << QT_VERSION_STR
             << "Import paths:" << engine.importPathList();


    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("CompressToFit", "Main");

    return QGuiApplication::exec();
}