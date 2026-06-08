#include <mio/mmap.hpp>

import util;
#ifdef _WIN32
#include <windows.h>
#endif
#include "src/util/macros.hpp"

import parser;
import lz77;
import util;
import std.compat;

namespace fs = std::filesystem;


int main(int argc, char* argv[])
{
	WIN_CALL(SetConsoleOutputCP(CP_UTF8));


	return 0;
}
