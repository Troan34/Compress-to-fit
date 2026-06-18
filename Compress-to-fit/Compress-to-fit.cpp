
#include <mio/mmap.hpp>
#include "src/util/macros.hpp"

import util;

import parser;
import models;
import util;
import std.compat;

namespace fs = std::filesystem;


int main(int argc, char* argv[])
{
	WIN_CALL(SetConsoleOutputCP(CP_UTF8));

	auto const options = parser::parse(argc, argv);

	process_file(options);

	return 0;
}
