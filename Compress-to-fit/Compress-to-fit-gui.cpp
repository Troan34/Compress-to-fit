#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QQmlContext>
#include <QDebug>
#include "src/gui/interface.hpp"

import util;

import parser;
import models;
import util;
import std.compat;

namespace fs = std::filesystem;


int main(int argc, char* argv[])
{

    qDebug() << QCoreApplication::applicationName()
             << "Qt version:" << QT_VERSION_STR;
    QGuiApplication app(argc, argv);

    CompressConfig compressor_conf;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("compressor_conf", &compressor_conf);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("CompressToFit", "Main");

    return QGuiApplication::exec();
}