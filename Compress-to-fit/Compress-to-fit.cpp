
#include <mio/mmap.hpp>
#include "src/util/macros.hpp"
#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>

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
	QObject::connect(
		&engine,
		&QQmlApplicationEngine::objectCreationFailed,
		&app,
		[]() { QCoreApplication::exit(-1); },
		Qt::QueuedConnection);
	engine.loadFromModule("CompressToFit", "Main");

	return QGuiApplication::exec();

	WIN_CALL(SetConsoleOutputCP(CP_UTF8));

	auto const options = parser::parse(argc, argv);

	process_file(options);

	return 0;
}
