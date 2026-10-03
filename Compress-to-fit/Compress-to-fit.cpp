/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Entry for the CLI program
*/
#include <cstdlib>

#include "src/common/error_warn_print.hpp"
#include "src/util/macros.hpp"
#include "src/common/sink.hpp"

import util;
import sink;
import parser;
import models;
import util;
import std.compat;

namespace fs = std::filesystem;


int main(int argc, char* argv[])
{
	WIN_CALL(SetConsoleOutputCP(CP_UTF8));

	//Set up the sink
	StdoutSink sink;
	SinkInstancer sink_instance{sink};

	try
	{
		auto const options = parser::parse(argc, argv);

		process_file(options);
	}
	catch (ErrorException const& e)
	{
		std::println(stderr, "{}", e.what());
		return static_cast<int>(e.error_type);
	}
	catch (parser::HelpException const& e)
	{
		std::println("{}", e.what());
		return 0;
	}
	catch (std::exception const& e)
	{
		std::println(stderr, "{}", e.what());
		return -1;
	}
	catch (...)
	{
		std::println(stderr,  "Unknown exception.");
		return -1;
	}


	return 0;
}
