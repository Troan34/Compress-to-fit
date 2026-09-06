#include <cstdlib>

#include "src/common/error_warn_print.hpp"
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

	try
	{
		auto const options = parser::parse(argc, argv);
		if (options.need_help)
		{
			return EXIT_SUCCESS;
		}

		process_file(options);
	}
	catch (ErrorException const& e)
	{
		std::cout << e.what() << '\n';
		return static_cast<int>(e.error_type);
	}
	catch (std::exception const& e)
	{
		return -1;
	}
	catch (...)
	{
		std::cout << "unknown error\n";
		return -1;
	}


	return 0;
}
