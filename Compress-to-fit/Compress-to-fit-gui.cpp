/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Entry for the GUI program
*/
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QQmlContext>
#include <QDebug>
#include "src/gui/interface.hpp"
#include "src/gui/fs_helper.hpp"
#include "src/gui/common.hpp"
#include "src/gui/sink.hpp"

import util;

import parser;
import models;
import util;
import std.compat;

namespace fs = std::filesystem;


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    GuiSink sink;
    engine.setExternalSingletonInstance("CompressToFit", "GuiSink", &sink);
    CompressConfig compressor_conf{sink, process_file};
    engine.rootContext()->setContextProperty("compressor_conf", &compressor_conf);
    FileSystem file_system;
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

    return QApplication::exec();
}
